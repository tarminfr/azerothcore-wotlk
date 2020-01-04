set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${AC_SCRIPTS_DIR}/Custom/sc_locklevel.cpp
  ${AC_SCRIPTS_DIR}/Custom/sc_locklevel.h
)

AC_ADD_SCRIPT_LOADER("Custom" "sc_locklevel.h")

message("  -> Prepared: CS LockLevel")
