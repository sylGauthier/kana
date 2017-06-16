#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "base.h"
#include "signature.h"

#define HEADER_MIN_SIZE 13

enum ReadStatus {
    ABORT,
    CONTINUE,
    END,
    EMPTY
};

static enum ReadStatus read_entry(struct Entry* entry, FILE* f) {
    long pos = ftell(f);
    long end;

    fseek(f, 0, SEEK_END);
    end = ftell(f);
    fseek(f, pos, SEEK_SET);

    if (end == pos) {
        return EMPTY;
    }

    if (!fseek(f, HEADER_MIN_SIZE + SIG_LEN, SEEK_CUR)) {
        unsigned int i;
        fseek(f, pos, SEEK_SET);
        entry->fileNameLen = 0;
        entry->width = 0;
        entry->height = 0;

        for (i = 0; i < 3; i++) {
            entry->width = (entry->width + fgetc(f)) << 8;
        }
        entry->width += fgetc(f);

        for (i = 0; i < 3; i++) {
            entry->height = (entry->height + fgetc(f)) << 8;
        }
        entry->height += fgetc(f);

        for (i = 0; i < 3; i++) {
            entry->fileNameLen = (entry->fileNameLen + fgetc(f)) << 8;
        }
        entry->fileNameLen += fgetc(f);

        pos = ftell(f);

        if (!fseek(f, entry->fileNameLen + SIG_LEN, SEEK_CUR) && entry->fileNameLen < FILE_LEN) {
            enum ReadStatus status;

            fseek(f, pos, SEEK_SET);
            fgets(entry->fileName, entry->fileNameLen + 1, f);
            for (i = 0; i < SIG_LEN; i++) {
                entry->signature[i] = getc(f);
            }
            entry->sigLen = SIG_LEN;
            pos = ftell(f);
            fseek(f, 0, SEEK_END);
            status = (pos == ftell(f)) ? END : CONTINUE;
            fseek(f, pos, SEEK_SET);

            return status;
        }
    }
    fprintf(stderr, "Corrupted base, inconsistent file length\n");
    return ABORT;
}

static int append_entry(struct Base* base, struct Entry* entry) {
    if ((base->data = realloc(base->data, (base->size + 1) * sizeof(struct Entry)))) {
        memcpy(base->data + base->size, entry, sizeof(struct Entry));
        base->size ++;
        return 0;
    } else {
        fprintf(stderr, "Can't allocate more memory for data base, aborting\n");
        return -1;
    }
}

int load_base(struct Base* base, char* filename) {
    struct Entry curEntry;

    if (!((base->file = fopen(filename, "r+")) || (base->file = fopen(filename, "w+")))) {
        fprintf(stderr, "Error, could not open nor create data base file\n");
        return -1;
    }

    base->data = NULL;
    base->size = 0;

    while (1) {
        switch (read_entry(&curEntry, base->file)) {
            case ABORT :
                fprintf(stderr, "Error: base loading failed\n");
                return -1;
                break;

            case END :
                if (append_entry(base, &curEntry) != 0)
                    return -1;
                printf("%d entries loaded succesfully\n", base->size);
                base->appendPos = base->size;
                return 0;
                break;

            case CONTINUE :
                if (append_entry(base, &curEntry) != 0)
                    return -1;
                break;

            case EMPTY :
                printf("Loaded empty base\n");
                base->appendPos = base->size;
                return 0;
                break;
        }
    }

    return -1;
}

int save_base(struct Base* base) {
    int i;

    printf("Saving data base of size %d...\n", base->size);
    fseek(base->file, 0, SEEK_SET);

    for (i = 0; i < base->size; i++) {
        int j;

        /*Writing width*/
        for (j = 0; j < 4; j++) {
            if (fputc((unsigned char) (base->data[i].width >> (8*(3-j))), base->file) == EOF) {
                fprintf(stderr, "Error writing file name length in data base file, aborting, data base may be corrupted\n");
                return -1;
            }
        }

        /*Writing height*/
        for (j = 0; j < 4; j++) {
            if (fputc((unsigned char) (base->data[i].height >> (8*(3-j))), base->file) == EOF) {
                fprintf(stderr, "Error writing file name length in data base file, aborting, data base may be corrupted\n");
                return -1;
            }
        }

        /*Writing file name length*/
        for (j = 0; j < 4; j++) {
            if (fputc((unsigned char) (base->data[i].fileNameLen >> (8*(3-j))), base->file) == EOF) {
                fprintf(stderr, "Error writing file name length in data base file, aborting, data base may be corrupted\n");
                return -1;
            }
        }

        /*Writing file name*/
        if (fputs(base->data[i].fileName, base->file) == EOF) {
            fprintf(stderr, "Error writing file name in data base, aborting, data base may be corrupted\n");
            return -1;
        }

        /*Writing signature*/
        for (j = 0; j < base->data[i].sigLen; j++) {
            if (fputc(base->data[i].signature[j], base->file) == EOF) {
                fprintf(stderr, "Error writing signature in data base, aborting, data base may be corrupted\n");
            }
        }
    }

    printf("Done\n");

    base->appendPos = base->size;
    return 0;
}

void close_base(struct Base* base) {
    printf("Closing data base\n");
    fclose(base->file);
    free(base->data);
}

int add_image_to_base(struct Base* base, struct Image* image) {
    struct Entry newEntry;
    int i;

    printf("Adding %s\n", image->fileName);
    newEntry.sigLen = SIG_LEN;
    newEntry.fileNameLen = strlen(image->fileName);
    newEntry.width = image->width;
    newEntry.height = image->height;
    strcpy(newEntry.fileName, image->fileName);

    if (signature_length(SIG_DEPTH) > SIG_LEN) {
        fprintf(stderr,
                "Error: pre-defined signature depth of %d would overflow the pre-defined signature length (%d)\n", SIG_DEPTH, SIG_LEN);
        return -1;
    }

    compute_signature(image, SIG_DEPTH, newEntry.signature);
    
    for (i = 0; i < base->size; i++) {
        float dist = distance(newEntry.signature, base->data[i].signature, SIG_LEN, SIG_DEPTH);

        if (dist < DISTANCE_THRESHOLD) {
            char choice;

            printf("Close match detected: %s (old) and %s (new)\n", base->data[i].fileName, newEntry.fileName);
            printf("Dimensions: %dx%d (old) %dx%d (new)\n", base->data[i].width, base->data[i].height, newEntry.width, newEntry.height);
            printf("Distance: %f\n", dist);
            printf("Add anyway: a, Replace: r, Skip: S\n");

            choice = getchar();

            switch (choice) {
                case 'a' :
                    getchar();
                    printf("Appending\n");

                    return append_entry(base, &newEntry);
                    break;

                case 'r' :
                    getchar();
                    printf("Replacing\n");
                    base->data[i] = newEntry;
                    return 0;
                    break;

                case '\n' :
                    printf("Skipping\n");
                    return 0;
                    break;

                default :
                    getchar();
                    printf("Skipping\n");
                    return 0;
                    break;

            }
        }
    }

    return append_entry(base, &newEntry);
}
