file(REMOVE_RECURSE
  "libindex.a"
  "libindex.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/index.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
