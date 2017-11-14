cmake .. \
    -DCMAKE_C_COMPILER="mpicc" \
    -DCMAKE_C_FLAGS="" \
    -DCMAKE_CXX_COMPILER="mpicxx" \
    -DCMAKE_CXX_FLAGS="-std=c++11" \
    -DENABLE_ZOLTAN=OFF \
    -DENABLE_PUMI=OFF \
    -DSCOREC_PREFIX=/path/to/core/install \
    -DENABLE_KOKKOS=OFF \
    -DKOKKOS_PREFIX=/path/to/kokkos_install \
    -DIS_TESTING=ON \
    -DMESHES=$PWD/../pumi-meshes \
    -DGRAPHS=$PWD/../EnGPar-graphs \

