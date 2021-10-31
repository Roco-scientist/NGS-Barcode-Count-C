# barcode-count
### work in progress
Currently of refactoring Rust code to C++

## Requirements
<ul>
<li>conan</li>
</ul>

## Building
```
mkdir build && cd build
conan install ..
cd ..
cmake CMakeList.txt
make
```

## Run
```
bin/barcode-count -c <counted_barcode_file> -s <sample_barcode_file> -q <format_file>
```
