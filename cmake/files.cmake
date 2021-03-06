set(PROJECT_SOURCE_FILES
    "${PROJECT_SRC_DIR}/main.cpp"
    "${PROJECT_SRC_DIR}/application.cpp"
    "${PROJECT_SRC_DIR}/link/mqtt.cpp"
    "${PROJECT_SRC_DIR}/link/database.cpp"
    "${PROJECT_SRC_DIR}/messages/event.cpp"
    "${PROJECT_SRC_DIR}/messages/detectorlog.cpp"
    "${PROJECT_SRC_DIR}/utility/threadrunner.cpp"
    "${PROJECT_SRC_DIR}/utility/log.cpp"
    "${PROJECT_SRC_DIR}/utility/utility.cpp"
    "${PROJECT_SRC_DIR}/utility/restservice.cpp"
    "${PROJECT_SRC_DIR}/utility/scopeguard.cpp"
    "${PROJECT_SRC_DIR}/utility/configuration.cpp"
    "${PROJECT_SRC_DIR}/utility/exceptions.cpp"
    "${PROJECT_SRC_DIR}/analysis/simplecoincidence.cpp"
    "${PROJECT_SRC_DIR}/analysis/coincidence.cpp"
    "${PROJECT_SRC_DIR}/analysis/eventconstructor.cpp"
    "${PROJECT_SRC_DIR}/analysis/coincidencefilter.cpp"
    "${PROJECT_SRC_DIR}/analysis/detectorstation.cpp"
    "${PROJECT_SRC_DIR}/analysis/stationcoincidence.cpp"
    "${PROJECT_SRC_DIR}/supervision/state.cpp"
    "${PROJECT_SRC_DIR}/supervision/timebase.cpp"
    "${PROJECT_SRC_DIR}/supervision/resource.cpp"
    "${PROJECT_SRC_DIR}/supervision/station.cpp")

set(PROJECT_HEADER_FILES
    "${PROJECT_HEADER_DIR}/application.h"
    "${PROJECT_HEADER_DIR}/link/database.h"
    "${PROJECT_HEADER_DIR}/link/mqtt.h"
    "${PROJECT_HEADER_DIR}/sink/base.h"
    "${PROJECT_HEADER_DIR}/sink/database.h"
    "${PROJECT_HEADER_DIR}/sink/mqtt.h"
    "${PROJECT_HEADER_DIR}/sink/ascii.h"
    "${PROJECT_HEADER_DIR}/source/base.h"
    "${PROJECT_HEADER_DIR}/source/mqtt.h"
    "${PROJECT_HEADER_DIR}/pipeline/base.h"
    "${PROJECT_HEADER_DIR}/messages/event.h"
    "${PROJECT_HEADER_DIR}/messages/detectorlog.h"
    "${PROJECT_HEADER_DIR}/messages/detectorinfo.h"
    "${PROJECT_HEADER_DIR}/messages/detectorsummary.h"
    "${PROJECT_HEADER_DIR}/messages/clusterlog.h"
    "${PROJECT_HEADER_DIR}/messages/userinfo.h"
    "${PROJECT_HEADER_DIR}/messages/trigger.h"
    "${PROJECT_HEADER_DIR}/messages/detectorstatus.h"
    "${PROJECT_HEADER_DIR}/utility/threadrunner.h"
    "${PROJECT_HEADER_DIR}/utility/log.h"
    "${PROJECT_HEADER_DIR}/utility/utility.h"
    "${PROJECT_HEADER_DIR}/utility/geohash.h"
    "${PROJECT_HEADER_DIR}/utility/restservice.h"
    "${PROJECT_HEADER_DIR}/utility/base64.h"
    "${PROJECT_HEADER_DIR}/utility/scopeguard.h"
    "${PROJECT_HEADER_DIR}/utility/exceptions.h"
    "${PROJECT_HEADER_DIR}/utility/coordinatemodel.h"
    "${PROJECT_HEADER_DIR}/utility/units.h"
    "${PROJECT_HEADER_DIR}/utility/configuration.h"
    "${PROJECT_HEADER_DIR}/analysis/dataseries.h"
    "${PROJECT_HEADER_DIR}/analysis/cachedvalue.h"
    "${PROJECT_HEADER_DIR}/analysis/ratemeasurement.h"
    "${PROJECT_HEADER_DIR}/analysis/histogram.h"
    "${PROJECT_HEADER_DIR}/analysis/simplecoincidence.h"
    "${PROJECT_HEADER_DIR}/analysis/coincidence.h"
    "${PROJECT_HEADER_DIR}/analysis/criterion.h"
    "${PROJECT_HEADER_DIR}/analysis/uppermatrix.h"
    "${PROJECT_HEADER_DIR}/analysis/eventconstructor.h"
    "${PROJECT_HEADER_DIR}/analysis/coincidencefilter.h"
    "${PROJECT_HEADER_DIR}/analysis/detectorstation.h"
    "${PROJECT_HEADER_DIR}/analysis/stationcoincidence.h"
    "${PROJECT_HEADER_DIR}/supervision/state.h"
    "${PROJECT_HEADER_DIR}/supervision/timebase.h"
    "${PROJECT_HEADER_DIR}/supervision/resource.h"
    "${PROJECT_HEADER_DIR}/supervision/station.h")
