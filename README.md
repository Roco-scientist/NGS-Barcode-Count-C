# barcode-count
C++ refactoring of <a href="https://github.com/Roco-scientist/NGS-Barcode-Count-dummy">NGS-Barcode-Count</a>.  Currently not as versatile as NGS-Barcode-Count, which is written in Rust.
<br>
<br>
Fast and memory efficient DNA barcode counter and decoder for next generation sequencing data.  Includes error handling.  Works for DEL (DNA encoded libraries), high throughput CRISPR sequencing, barcode sequencing.  If the barcode file is included, the program will convert to barcode names and correct for errors. If a random barcode is included to collapse PCR duplicates, these duplicates will not be counted.  Parsing over 400 million sequencing reads took under a half hour with 8 threads and around 2GB of RAM use.<br>
<br>
<br>
For DEL analysis, a companion python package was created: <a href=https://github.com/Roco-scientist/DEL-Analysis>DEL-Analysis</a>
<br>
<br>
Multithreaded and low resource use.  Uses one thread to read and the rest to process the data, so at least a 2 threaded machine is essential.
This program does not store all data within RAM but instead sequentially processes the sequencing data in order to remain memory efficient.  
<br>
<br>
Error handling is defaulted at 20% maximum sequence error per constant region and barcode.  The algorithm fixes any sequenced constant region or barcode with the best match possible.  If there are two or more best matches,
it is not counted.
<br>
<br>
If there is a random barcode included, sequences with a duplicated random barcode are not counted.
<br>
<br>
Inspired by and some ideas adopted from <a href=https://github.com/sunghunbae/decode target="_blank" rel="noopener noreferrer">decode</a>


## Requirements
<ul>
<li>cmake</li>
<li>zlib installed</li>
</ul>

## Build from source
```
git clone https://github.com/Roco-scientist/barcode-count.git
cd barcode-count
cmake .
cmake --build .
```

## Files Needed
Currently supports FASTQ, sequence format, sample barcode conversion, and building block barcode conversion.
<ul>
<li><a href=#fastq-file>FASTQ</a></li>
<li><a href=#sequence-format-file>Sequence format file</a></li>
<li><a href=#sample-barcode-file>Sample barcode file</a></li>
<li><a href=#counted-barcode-conversion-file>Counted barcode conversion file</a></li>
</ul>


### Fastq File
Accepts gzipped and unzipped fastq files.<br>

### Sequence Format File
The sequence format file should be a text file that is line separated by the type of format.  The following is supported where the '#' should be replaced by the number of nucleotides corresponding to the barcode:<br>
<table>
<tr>
<th>Sequence Type</th>
<th>File Code</th>
<th>Number Needed/Allowed</th>
</tr>
<td>Constant</td>
<td>ATGCN</td>
<td>1 or more</td>
<tr>
<td>Sample Barcode</td>
<td>[#]</td>
<td>0-1</td>
</tr>
<tr>
<td>Barcode for counting</td>
<td>{#}</td>
<td>1 or more</td>
</tr>
<tr>
<td>Random Barcode</td>
<td>(#)</td>
<td>0-1</td>
</tr>
</table>

An example can be found in [scheme.example.txt](scheme.example.txt).  Since the algorthm uses a regex search to find the scheme, the scheme can exist anywhere within the sequence read.

### Sample Barcode File
<b>Optional</b><br>
The sample_barcode_file is a comma separate file with the following format:<br>
<table>
<tr>
<th>Barcode</th>
<th>Sample_ID</th>
</tr>
<tr>
<td>AGCATAC</td>
<td>Sample_name_1</td>
</tr>
<tr>
<td>AACTTAC</td>
<td>Sample_name_2</td>
</tr>
</table>

An example can be found in [sample_barcode.example.csv](sample_barcode.example.csv).

### Counted Barcode Conversion File
<b>Optional</b><br>
The barcode_file is a comma separate file with the following format:<br>
<table>
<tr>
<th>Barcode</th>
<th>Barcode_ID</th>
<th>Barcode_Number</th>
</tr>
<tr>
<td>CAGAGAC</td>
<td>Barcode_name_1</td>
<td>1</td>
</tr>
<tr>
<td>TGATTGC</td>
<td>Barcode_name_2</td>
<td>1</td>
</tr>
<tr>
<td>ATGAAAT</td>
<td>Barcode_name_3</td>
<td>2</td>
</tr>
<tr>
<td>GCGCCAT</td>
<td>Barcode_name_4</td>
<td>2</td>
</tr>
<tr>
<td>GATAGCT</td>
<td>Barcode_name_5</td>
<td>3</td>
</tr>
<tr>
<td>TTAGCTA</td>
<td>Barcode_name_6</td>
<td>3</td>
</tr>
</table>

An example can be found in [barcode.example.csv](barcode.example.csv).<br><br>

Where the first column is the DNA barcode, the second column is the barcode ID which can be a smile string for DEL, CRISPR target ID, etc. but cannot contain commas. 
The last column is the barcode number as an integer.  The barcode numbers are in the same order as the sequence format file and starting
at 1. For example, if there are a total of 3 barcodes, which may be the case with DEL, you would only have 1, 2, or 3 within this column for each row, with each number
representing one of the three barcodes. For CRISPR or barcode seq, where there may only be one barcode to count, this column would be all 1s.

## Run
After compilation, the `barcode` binary can be moved anywhere.
<br>
<br>
Run barcode-count<br>

```
bin/barcode-count --fastq <fastq_file> \
	--sample_barcodes <sample_barcodes_file> \
	--sequence_format <sequence_format_file> \
	--counted_barcodes <counted_barcodes_file> \
	--output_dir <output_dir> \
	--threads <num_of_threads> \
	--merge_output \
```

<br>
<ul>
<li>
--counted_barcodes is optional.  If it is not used, the output counts uses the DNA barcode to count with no error handling on these barcodes.
</li>
<li>
--sample_barcodes is optional.  
</li>
<li>
--output_dir defaults to the current directory if not used.
</li>
<li>
--threads defaults to the number of cores on the machine.
</li>
<li>
--merge_output flag that merges the output csv file so that each sample has one column
</li>
</ul>

### Output files
Each sample name will get a file in the default format of year-month-day_<sample_name>_counts.csv in the following format (for 3 building blocks):
<table>
<tr>
<th>Barcode_1</th>
<th>Barcode_2</th>
<th>Barcode_3</th>
<th>Count</th>
</tr>
<tr>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>#</td>
</tr>
<tr>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>#</td>
</tr>
</table>

Where Barcode_ID is used if there is a building block conversion file, otherwise the DNA code is used. `#` represents the count number<br><br>
If `--merge_output` is called, an additional file is created with the format (for 3 samples):

<table>
<tr>
<th>Barcode_1</th>
<th>Barcode_2</th>
<th>Barcode_3</th>
<th>Sample_1</th>
<th>Sample_2</th>
<th>Sample_3</th>
</tr>
<tr>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>#</td>
<td>#</td>
<td>#</td>
</tr>
<tr>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>Barcode_ID/DNA code</td>
<td>#</td>
<td>#</td>
<td>#</td>
</tr>
</table>

## Tests results
On an 8 threaded i7-4790K CPU @ 4.00GHz with 16gb RAM, this algorithm was able to decode over 400 million sequencing reads in about a half hour.
Results below:
```
Total reads:            418770347
Counted:                257807865
Constant region errors: 151955695
Sample barcode errors:  3270100
Counted barcode errors: 5736687
Parsing time: 25 minutes 39.343 seconds
Writing
Total time: 32 minutes 45.029 seconds
```
