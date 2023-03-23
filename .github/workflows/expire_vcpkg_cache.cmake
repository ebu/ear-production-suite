# prune the VCPKG cache size based on the modification time of the files

set(cache "$ENV{VCPKG_DEFAULT_BINARY_CACHE}")
set(max_cache_size 500000000)

# find files in the cache
file(GLOB_RECURSE files LIST_DIRECTORIES false "${cache}/*")

# total size of all files seen in bytes
set(total_size 0)

# list of files with their unix timestamps separated by a space for sorting
set(timestamps "")

foreach(file IN LISTS files)
    file(TIMESTAMP "${file}" timestamp "%s" UTC)
    file(SIZE "${file}" size)

    list(APPEND timestamps "${timestamp} ${file}")
    math(EXPR total_size "${total_size} + ${size}")
endforeach()

message("vcpkg cache size before pruning: ${total_size}")

list(SORT timestamps COMPARE NATURAL ORDER ASCENDING)

# drop files until the total is less than the maximum size
foreach(timestamp_file IN LISTS timestamps)
    string(REPLACE " " ";" timestamp_file_list ${timestamp_file})
    list(GET timestamp_file_list 0 timestamp)
    list(GET timestamp_file_list 1 file)

    if (total_size LESS max_cache_size)
        break()
    endif()

    file(SIZE "${file}" size)
    math(EXPR total_size "${total_size} - ${size}")
    file(REMOVE ${file})

    message("deleted ${file}")
endforeach()

message("vcpkg cache size after pruning: ${total_size}")
