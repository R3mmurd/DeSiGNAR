# Doxygen configuration for DeSiGNAR (Spanish edition / edición en español).
#
# Same settings as Doxyfile, except: the main/landing page is README.es.md
# instead of README.md, output goes to es-doc/ instead of en-doc/, and
# Doxygen's own generated UI strings (menu labels like "Class List",
# "Public Functions", etc.) are in Spanish via OUTPUT_LANGUAGE. The
# source code's own Doxygen comments are NOT translated — they stay
# English throughout, matching this library's established convention —
# so class/function reference pages look the same in both editions;
# only the getting-started/installation main page is bilingual.
#
# Generate with:
#
#   doxygen Doxyfile.es
#
# HTML output is written to es-doc/html/index.html.

PROJECT_NAME           = "DeSiGNAR"
PROJECT_BRIEF           = "Una biblioteca de estructuras de datos y algoritmos en C++ con fines didacticos"
PROJECT_LOGO            = logo.png
OUTPUT_DIRECTORY        = es-doc
OUTPUT_LANGUAGE         = Spanish

INPUT                   = include README.es.md
FILE_PATTERNS           = *.hpp
RECURSIVE               = YES
USE_MDFILE_AS_MAINPAGE  = README.es.md

# @see Doxyfile's matching comment.
IMAGE_PATH              = docs
HTML_HEADER             = docs/doxygen-header-es.html

EXTRACT_ALL             = YES
EXTRACT_PRIVATE         = NO
EXTRACT_STATIC          = YES

DISTRIBUTE_GROUP_DOC    = YES

GENERATE_HTML           = YES
GENERATE_LATEX          = NO

WARN_IF_UNDOCUMENTED    = YES
WARN_IF_DOC_ERROR       = YES
WARN_NO_PARAMDOC        = NO

QUIET                   = YES
