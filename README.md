# Kana

## Description
This program computes distances between two images in a very fast and effective way. To do so, it computes a signature from each picture, that has the property of being continous, meaning a small change in the picture will induce a small change in the signature. From there, a distance between two signatures is easily defined and computed, and gives accurate result.

The program can also build a data baae of signatures/file association, and perform check for similar-looking pictures when adding a a new picture. It is useful to remove doubles from large dataset, and keep only the best quality version.

The major property of the algorithm in use is that it doesn't depend on the scale of the image. A 600*300 picture and the same picture resized to 200*500 for example will have a distance close to zero. A slightly cropped picture will also appear very close.

NB: For now, only jpeg files are supported. Adding png support is top priority.

## Usage
Once the repository is cloned :

```
cd kana
make
```

Then use the -h option (or anything not valid for that matter) to get the manual.

Example, let's say you have a folder A containing a large number of .jpg pictures and you want to remove doubles.

```
cd A
touch database.kana
kana add database.kana *.jpg
```

The program will automatically perform a distance check between pictures when it adds them to the database. In case of close match, you will be prompted to either add the similar-looking picture to the database anyway, skip it, or replace it. No file will be physically removed. The option may be added in the near future though.

## Algorithm description

Signature computation is performed with a very naive yet surprisingly effective algorithm. The picture is recursively split into four quadrants, each of them split again in four and so on, and each quadrant stores the mean of each component of all the pixels it covers. The signature is just the concatenation of all those means, from the biggest quadrant (level 0) to the smallest.

Therefore, false positive are a relatively common occurence when distance threshold is too high, but false negative almost never occurs.
