QT += widgets gui core network

CONFIG+= c++1z

SOURCES += \
    main.cpp \
    gui.cpp \
    alg.cpp \
    camera.cpp \
    registrausuario.cpp \
    cadastro.cpp \
    Chinese/chinesecluster.cpp \
    Chinese/pessoa.cpp

FORMS += \
    gui.ui \
    cadastro.ui

HEADERS += \
    gui.h \
    alg.h \
    camera.h \
    cadastro.h \
    imports.h \
    Chinese/chinesecluster.h \
    Chinese/pessoa.h \
    Chinese/ball.h \
    registrausuario.h

QMAKE_CXXFLAGS_DEBUG += -g -Og -dA -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wdiv-by-zero -Wendif-labels -Wformat-extra-args -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wmissing-declarations -Wno-missing-format-attribute -Wmissing-include-dirs -Wmultichar -Wpacked -Wpointer-arith -Wreturn-type -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch -Wswitch-default -Werror=undef -Wno-unused -Wvariadic-macros -Wwrite-strings  -Werror=declaration-after-statement -Werror=implicit-function-declaration -Werror=nested-externs -Werror=strict-prototypes -fno-strict-aliasing -static #-Werror=missing-braces
#-fprofile-arcs -ftest-coverage
#QMAKE_LFLAGS_DEBUG += -lgcov --coverage

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O3


unix {
    #QMAKE_CXXFLAGS_RELEASE += -Os -s -fPIE -fvisibility-inlines-hidden -flto -fomit-frame-pointer -fstack-protector-all -D_FORTIFY_SOURCE=2 -fcf-protection=full -fstack-clash-protection -fvisibility=hidden -fsanitize=address -fsanitize=kernel-address -fsanitize-address-use-after-scope -fsanitize-coverage=trace-cmp -mmpx -fcheck-pointer-bounds -static -fstack-check -fstack-clash-protection -fsplit-stack -fvtable-verify=preinit -fstack-reuse=none -fno-exceptions -freg-struct-return -fno-jump-tables -mfpmath="387+sse" ## HARDENING
    #QMAKE_LFLAGS += -flto -Wl,-z,now -Wl,-z,relro #HARDEN

    QMAKE_CXXFLAGS_RELEASE += -Ofast -fwhole-program -fomit-frame-pointer -fmodulo-sched -fmodulo-sched-allow-regmoves -fgcse-sm -fgcse-las -fgcse-after-reload -funsafe-loop-optimizations -flive-range-shrinkage -fsched-spec-load-dangerous -fsched2-use-superblocks -floop-nest-optimize -floop-parallelize-all -ftree-parallelize-loops=8 -fprefetch-loop-arrays -ffinite-math-only -march=native -mtune=native -mfpmath="387+sse" -std=c++1z -static

    INCLUDEPATH += /usr/local/cuda/include
    INCLUDEPATH += /opt/intel/mkl/include
    INCLUDEPATH += /usr/local/include/opencv
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/include

    LIBS += -L/usr/lib \
    -lblas \
    -llapack

    LIBS += -ldl -lrt

    LIBS += -L/opt/intel/mkl/lib/intel64 -lmkl_rt -lmkl_core

    LIBS += -L/usr/local/lib -ldlib

    LIBS += -L/usr/local/cuda/lib64/ -lcudnn -lcudart -lcublas -lcurand -lcusolver

    LIBS += -ldl -lrt

    #LIBS += -L/usr/local/lib64 \
    #-lclBLAS


    LIBS += -L/usr/local/lib \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_imgcodecs \
    -lopencv_ml \
    -lopencv_video \
    -lopencv_videoio \
    -lopencv_videostab \
    -lopencv_objdetect \
    -lopencv_features2d \
    -lopencv_calib3d \
    -lopencv_objdetect \
    -lopencv_flann \
    -lopencv_optflow \
    -lopencv_bgsegm \
    -lopencv_cudaimgproc \
    -lopencv_cudawarping \
    -lopencv_cudafilters \
    -lopencv_cudaarithm \
    -lopencv_cudaobjdetect \
    -lopencv_photo \
    -lopencv_xphoto
#    -lopencv_cudacodec \
#    -lopencv_cudafeatures2d \
#    -lopencv_cudafilters \
#    -lopencv_cudastereo \
#    -lopencv_cudalegacy

#   -lopencv_cudabgmsegm

    LIBS += -L/usr/lib -lomp

}

win32 { #sem suporte cuda
    QMAKE_CXXFLAGS_RELEASE += -Ofast -fomit-frame-pointer -fmodulo-sched -fmodulo-sched-allow-regmoves -fgcse-sm -fgcse-las -fgcse-after-reload -funsafe-loop-optimizations -flive-range-shrinkage -fsched-spec-load-dangerous -fsched2-use-superblocks -floop-nest-optimize -floop-parallelize-all -ftree-parallelize-loops=8 -fprefetch-loop-arrays -ffinite-math-only -march=native -mtune=native -mfpmath="387+sse" -std=c++1z
    INCLUDEPATH += E:\Programas\msys2\mingw64\include

    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_core341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_imgcodecs341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_imgproc341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_features2d341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_calib3d341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_dnn341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_photo341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_video341.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopencv_videoio341.dll


    LIBS += E:\Programas\msys2\mingw64\bin\libdlib.dll

    LIBS += E:\Programas\msys2\mingw64\bin\liblapack.dll
    LIBS += E:\Programas\msys2\mingw64\bin\libopenblas.dll

}

RESOURCES += \
    resource.qrc

DISTFILES += \
    TODO_LIST



