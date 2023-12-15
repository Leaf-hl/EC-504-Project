# EC-504-Project

This project implemented in C++ is used to find the K nearest neighbor state/county in the US for one specified coordinate. 

## Input Format

This system will first load a huge number of reference points in the US. Then it will allow the user to specify one location by inputting its latitude and longitude. K, the quantity of nearest neighbors which should be a number from 1 to 10, is also required for the user to assign.

## Output Format

The system will return results with two methods: KD-Tree searching and linear scanning. The corresponding time consumed will also be shown.

## How to run

- First Generate the executable file using `make` command and it will create a file called `main`
- Then in the command line, type as following:
```shell
main $latitude $langitude
```
