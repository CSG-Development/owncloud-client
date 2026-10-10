# SPDX-License-Identifier: 0BSD
TEMPLATE = lib
TARGET = lzma
CONFIG += staticlib
INCLUDEPATH += $$PWD $$PWD/src/liblzma/common $$PWD/src/common $$PWD/src/liblzma/api $$PWD/src/liblzma/check $$PWD/src/liblzma/delta $$PWD/src/liblzma/lz $$PWD/src/liblzma/lzma $$PWD/src/liblzma/rangecoder $$PWD/src/liblzma/simple

include(../../../../installerfw.pri)
DESTDIR = $$IFW_LIB_PATH

DEFINES += HAVE_CONFIG_H LZMA_API_STATIC WIN32 _CRT_SECURE_NO_WARNINGS

SOURCES += \
    $$PWD/src/common/tuklib_cpucores.c \
    $$PWD/src/common/tuklib_physmem.c \
    $$PWD/src/liblzma/check/check.c \
    $$PWD/src/liblzma/check/crc32_fast.c \
    $$PWD/src/liblzma/check/crc32_table.c \
    $$PWD/src/liblzma/check/crc64_fast.c \
    $$PWD/src/liblzma/check/crc64_table.c \
    $$PWD/src/liblzma/check/sha256.c \
    $$PWD/src/liblzma/common/alone_decoder.c \
    $$PWD/src/liblzma/common/alone_encoder.c \
    $$PWD/src/liblzma/common/auto_decoder.c \
    $$PWD/src/liblzma/common/block_buffer_decoder.c \
    $$PWD/src/liblzma/common/block_buffer_encoder.c \
    $$PWD/src/liblzma/common/block_decoder.c \
    $$PWD/src/liblzma/common/block_encoder.c \
    $$PWD/src/liblzma/common/block_header_decoder.c \
    $$PWD/src/liblzma/common/block_header_encoder.c \
    $$PWD/src/liblzma/common/block_util.c \
    $$PWD/src/liblzma/common/common.c \
    $$PWD/src/liblzma/common/easy_buffer_encoder.c \
    $$PWD/src/liblzma/common/easy_decoder_memusage.c \
    $$PWD/src/liblzma/common/easy_encoder_memusage.c \
    $$PWD/src/liblzma/common/easy_encoder.c \
    $$PWD/src/liblzma/common/easy_preset.c \
    $$PWD/src/liblzma/common/file_info.c \
    $$PWD/src/liblzma/common/filter_buffer_decoder.c \
    $$PWD/src/liblzma/common/filter_buffer_encoder.c \
    $$PWD/src/liblzma/common/filter_common.c \
    $$PWD/src/liblzma/common/filter_decoder.c \
    $$PWD/src/liblzma/common/filter_encoder.c \
    $$PWD/src/liblzma/common/filter_flags_decoder.c \
    $$PWD/src/liblzma/common/filter_flags_encoder.c \
    $$PWD/src/liblzma/common/hardware_cputhreads.c \
    $$PWD/src/liblzma/common/hardware_physmem.c \
    $$PWD/src/liblzma/common/index_decoder.c \
    $$PWD/src/liblzma/common/index_encoder.c \
    $$PWD/src/liblzma/common/index_hash.c \
    $$PWD/src/liblzma/common/index.c \
    $$PWD/src/liblzma/common/lzip_decoder.c \
    $$PWD/src/liblzma/common/microlzma_decoder.c \
    $$PWD/src/liblzma/common/microlzma_encoder.c \
    $$PWD/src/liblzma/common/outqueue.c \
    $$PWD/src/liblzma/common/stream_buffer_decoder.c \
    $$PWD/src/liblzma/common/stream_buffer_encoder.c \
    $$PWD/src/liblzma/common/stream_decoder_mt.c \
    $$PWD/src/liblzma/common/stream_decoder.c \
    $$PWD/src/liblzma/common/stream_encoder_mt.c \
    $$PWD/src/liblzma/common/stream_encoder.c \
    $$PWD/src/liblzma/common/stream_flags_common.c \
    $$PWD/src/liblzma/common/stream_flags_decoder.c \
    $$PWD/src/liblzma/common/stream_flags_encoder.c \
    $$PWD/src/liblzma/common/string_conversion.c \
    $$PWD/src/liblzma/common/vli_decoder.c \
    $$PWD/src/liblzma/common/vli_encoder.c \
    $$PWD/src/liblzma/common/vli_size.c \
    $$PWD/src/liblzma/delta/delta_common.c \
    $$PWD/src/liblzma/delta/delta_decoder.c \
    $$PWD/src/liblzma/delta/delta_encoder.c \
    $$PWD/src/liblzma/lz/lz_decoder.c \
    $$PWD/src/liblzma/lz/lz_encoder_mf.c \
    $$PWD/src/liblzma/lz/lz_encoder.c \
    $$PWD/src/liblzma/lzma/fastpos_table.c \
    $$PWD/src/liblzma/lzma/lzma_decoder.c \
    $$PWD/src/liblzma/lzma/lzma_encoder_optimum_fast.c \
    $$PWD/src/liblzma/lzma/lzma_encoder_optimum_normal.c \
    $$PWD/src/liblzma/lzma/lzma_encoder_presets.c \
    $$PWD/src/liblzma/lzma/lzma_encoder.c \
    $$PWD/src/liblzma/lzma/lzma2_decoder.c \
    $$PWD/src/liblzma/lzma/lzma2_encoder.c \
    $$PWD/src/liblzma/rangecoder/price_table.c \
    $$PWD/src/liblzma/simple/arm.c \
    $$PWD/src/liblzma/simple/arm64.c \
    $$PWD/src/liblzma/simple/armthumb.c \
    $$PWD/src/liblzma/simple/ia64.c \
    $$PWD/src/liblzma/simple/powerpc.c \
    $$PWD/src/liblzma/simple/simple_coder.c \
    $$PWD/src/liblzma/simple/simple_decoder.c \
    $$PWD/src/liblzma/simple/simple_encoder.c \
    $$PWD/src/liblzma/simple/sparc.c \
    $$PWD/src/liblzma/simple/x86.c
