set(TEMPLATE_FILE license.h.in)
set(HEADER_FILE license.h)
file(READ ../../../LICENSE.TXT LICENSE_CONTENT)

# Workaround max string length (C2026)- Split the long string at every 2000 bytes
string(LENGTH "${LICENSE_CONTENT}" LICENSE_CONTENT_LENGTH)
math(EXPR NUM_PARTS "${LICENSE_CONTENT_LENGTH} / 2000")
set(LICENSE_TEXT "")
foreach(i RANGE ${NUM_PARTS})
  math(EXPR START_POS "${i} * 2000")
  string(SUBSTRING "${LICENSE_CONTENT}" ${START_POS} 2000 PART)
  string(CONCAT LICENSE_TEXT ${LICENSE_TEXT} "\nR\"L1C3N53T3XT(" ${PART} ")L1C3N53T3XT\"")
endforeach()

configure_file(${TEMPLATE_FILE} ${HEADER_FILE})