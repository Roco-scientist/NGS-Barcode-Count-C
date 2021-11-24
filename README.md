# barcode-count
Fast and memory efficient DNA barcode counter and decoder for next generation sequencing data.
Includes error handling and a sequencing quality filter.  Works for DEL (DNA encoded libraries), high throughput CRISPR sequencing, barcode sequencing.
If the barcode file is included, the program will convert to barcode names and correct for errors.
If a random barcode is included to collapse PCR duplicates, these duplicates will not be counted.
Parsing over 400 million sequencing reads took under a half hour with 8 threads and around 2GB of RAM use.  
  
For DEL analysis, a companion python package was created: [DEL-Analysis](https://github.com/Roco-scientist/DEL-Analysis)  
  
Multithreaded and low resource use.  Uses one thread to read and the rest to process the data, so at least a 2 threaded machine is essential.
This program does not store all data within RAM but instead sequentially processes the sequencing data in order to remain memory efficient.    
  
Error handling is defaulted at 20% maximum sequence error per constant region and barcode. This can be changed through CLI arguments.
The algorithm fixes any sequenced constant region or barcode with the best match possible.  If there are two or more best matches,
it is not counted.  
  
~~Filtering by read quality score is also an option.  If used, each barcode has its read quality average calculated and if it is below the set threshold, the read is not counted.
The algorithm is defaulted to not filter unless the --min_quality argument is called.  See fastq documentation to understand read quality scores.
The scores used are after ascii conversion and 33 subtraction.~~  
  
C++ refactoring of [NGS-Barcode-Count](https://github.com/Roco-scientist/NGS-Barcode-Count-dummy), which is written in Rust. Features not yet refactored:  
- Stat file output
- Sequencing read quality filter  
  
Inspired by and some ideas adopted from [decode](https://github.com/sunghunbae/decode)  
  
## Requirements
- cmake (version >= 3.10)
- zlib (version 1.2.11 verified to work): sudo apt-get install zlib1g-dev

## Build from source

The following places a `barcode-count` executable within the `barcode-count/build/bin/` directory.  This executable can be moved anywhere.
One recommendation is to move it to a path directory (`echo $PATH`), such as `/usr/local/bin/` so that it can be executed from anywhere.
  
  
Clone the repo:

```
git clone https://github.com/Roco-scientist/barcode-count.git
cd barcode-count
```

Build the `barcode-count` executable within `./build/bin/`:

```
cmake -B ./build
cmake --build ./build
```

## Files Needed
Currently supports FASTQ, sequence format, sample barcode conversion, and building block barcode conversion.
- [FASTQ](#fastq-file)
- [Sequence format file](#sequence-format-file)
- [Sample barcode file (optional)](#sample-barcode-file)
- [Counted barcode conversion file (optional)](#counted-barcode-conversion-file)


### Fastq File
Accepts unzipped fastq files.  
Accepts gzipped fastq files, but if the program stops before the expected number of sequencing reads, unzip and rerun.

### Sequence Format File
The sequence format file should be a text file that is line separated by the type of format.  The following is supported where the '#' should be replaced by the number of nucleotides corresponding to the barcode:\
  
|Sequence Type|File Code|Number Needed/Allowed|
|-------------|---------|---------------------|
|Constant|ATGCN|1 or more|
|Sample Barcode|[#]|0-1|
|Barcode for counting|{#}|1 or more|
|Random Barcode|(#)|0-1|
  
An example can be found in [scheme.example.txt](scheme.example.txt).  Since the algorthm uses a regex search to find the scheme, the scheme can exist anywhere within the sequence read.

### Sample Barcode File
**Optional**  
The sample_barcode_file is a comma separate file with the following format:  
|Barcode|Sample_ID|
|-------|---------|
|AGCATAC|Sample_name_1|
|AACTTAC|Sample_name_2|
  
An example can be found in [sample_barcode.example.csv](sample_barcode.example.csv).

### Counted Barcode Conversion File
**Optional**  
The barcode_file is a comma separate file with the following format:  
|Barcode|Barcode_ID|Barcode_Number|
|-------|----------|--------------|
|CAGAGAC|Barcode_name_1|1|
|TGATTGC|Barcode_name_2|1|
|ATGAAAT|Barcode_name_3|2|
|GCGCCAT|Barcode_name_4|2|
|GATAGCT|Barcode_name_5|3|
|TTAGCTA|Barcode_name_6|3|
  
An example can be found in [barcode.example.csv](barcode.example.csv).
  
  
Where the first column is the DNA barcode, the second column is the barcode ID which can be a smile string for DEL, CRISPR target ID, etc. but cannot contain commas. 
The last column is the barcode number as an integer.  The barcode numbers are in the same order as the sequence format file and starting
at 1. For example, if there are a total of 3 barcodes, which may be the case with DEL, you would only have 1, 2, or 3 within this column for each row, with each number
representing one of the three barcodes. For CRISPR or barcode seq, where there may only be one barcode to count, this column would be all 1s.

## Run
After compilation, the `barcode-count` binary can be moved anywhere.  It will be located in barcode-count/build/bin directory after compilation.  
  
Run barcode-count  

```
build/bin/barcode-count --fastq <fastq_file> \
	--sample-barcodes <sample_barcodes_file> \
	--sequence-format <sequence_format_file> \
	--counted-barcodes <counted_barcodes_file> \
	--output-dir <output_dir> \
	--threads <num_of_threads> \
	--merge-output \
	--enrich
```

- --counted-barcodes is optional.  If it is not used, the output counts uses the DNA barcode to count with no error handling on these barcodes.
- --sample-barcodes is optional.  If it is not used, all samples are marked as unknown.
- --output-dir defaults to the current directory if not used.
- --prefix defaults to the current date.  All files end with _sample_name_counts.csv
- --threads defaults to the number of threads on the machine if not used.
- --merge-output flag that merges the output csv file so that each sample has one column
- --min-quality will filter out reads where any of the barcodes have an average quality score below the threshold set here.  Default is 0 and no filtering.
- --enrich argument flag that will find the counts for each barcode if there are 2 or more counted barcodes included, and output the file. Also will do the same with double barcodes if there are 3+. Useful for DEL

### Output files
Each sample name will get a file in the default format of year-month-day_<sample_name>_counts.csv in the following format (for 3 counted barcodes):

|Barcode_1|Barcode_2|Barcode_3|Count|
|---------|---------|---------|-----|
|Barcode_ID/DNA code|Barcode_ID/DNA code|Barcode_ID/DNA code|#|
|Barcode_ID/DNA code|Barcode_ID/DNA code|Barcode_ID/DNA code|#|

Where Barcode_ID is used if there is a counted barcode conversion file, otherwise the DNA code is used. `#` represents the count number<br><br>
If `--merge_output` is called, an additional file is created with the format (for 3 samples):

|Barcode_1|Barcode_2|Barcode_3|Sample_1|Sample_2|Sample_3|
|---------|---------|---------|--------|--------|--------|
|Barcode_ID/DNA code|Barcode_ID/DNA code|Barcode_ID/DNA code|#|#|#|
|Barcode_ID/DNA code|Barcode_ID/DNA code|Barcode_ID/DNA code|#|#|#|

An additional barcode_stats.txt file is also written/appended to the output folder.  This keeps track of running information.<br><br>
If the `--enrich` arguments is called, single and double barcode count files are ouptut.

## Uses

### DEL
Setup as shown with all example files used throughout this README.  Typically you will use 3 x '[]' for counting barcodes, which represents 3 building blocks, within the format file.

### CRISPR-seq
Same setup as with DEL, but typically with only one '[]' counted barcode in the format file.  As such, within the counted barcode conversion file, the third column will contain all '1's

### Barcode-seq
If the intention is to count the random barcodes and have the counts associated with these random barcodes, which is the case with bar-seq of cell pools for lineage evolution etc., 
then the random barcode, within this situation, is the counted barcode and represented with '[]' in the format file.  A counted barcode conversion file will not be included.  Without the counted barcode conversion file, 
the program will output the counted random barcode sequence and the associated count.  Afterwards, clustering or any other analysis can be applied.

## Tests results
On an 8 threaded i7-4790K CPU @ 4.00GHz with 16gb RAM, this algorithm was able to decode over 400 million sequencing reads in about a half hour.  
Results below:  
  
Inflated fastq file
```
Total reads:                 418770347
Correctly matched sequences: 257807865
Constant region mismatches:  151955695
Sample barcode mismatches:   3270100
Counted barcode mismatches:  5736687
Duplicates:                  0

Parsing time: 25 minutes 26.770 seconds

Writing 

Total time: 26 minutes 47.223 seconds
```

<br>
Gzipped fastq file

```
Total reads:                 418770347
Correctly matched sequences: 257807865
Constant region mismatches:  151955695
Sample barcode mismatches:   3270100
Counted barcode mismatches:  5736687
Duplicates:                  0

Parsing time: 25 minutes 57.438 seconds

Writing 

Total time: 27 minutes 17.314 seconds
```
