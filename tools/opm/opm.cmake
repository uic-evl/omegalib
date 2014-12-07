# Download an update to this and the rest of the opm scripts.
set(OPM_URL "https://raw.githubusercontent.com/uic-evl/omegalib/master/tools/opm")
file(DOWNLOAD ${OPM_URL}/opm.cmake ./opm.cmake SHOW_PROGRESS )
file(DOWNLOAD ${OPM_URL}/info.cmake ./opm/info.cmake SHOW_PROGRESS )
