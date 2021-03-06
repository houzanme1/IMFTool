find_file(asdcplib_Targets_PATH NAMES asdcplibtargets.cmake PATHS "$ENV{CMAKE_HINT}/asdcplib" ENV CMAKE_HINT PATH_SUFFIXES "targets")
find_file(OpenJPEG_Targets_PATH NAMES OpenJPEGTargets.cmake ENV CMAKE_HINT PATH_SUFFIXES "targets")
find_path(asdcplib_include_DIR NAMES AS_02.h PATHS "$ENV{CMAKE_HINT}/asdcplib" ENV CMAKE_HINT PATH_SUFFIXES "include")
find_path(OpenJPEG_include_DIR NAMES openjpeg.h)
find_library(OpenJPEGLib_Path NAMES openjp2 PATHS ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(OpenJPEGLib_Debug_Path NAMES openjp2 PATHS ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(XercescppLib_PATH NAMES xerces-c xerces-c_3 PATHS "${PROJECT_SOURCE_DIR}/../xercescpp" "${PROJECT_SOURCE_DIR}/../lib/xercescpp" "$ENV{CMAKE_HINT}/xercescpp" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(XercescppLib_Debug_PATH NAMES xerces-c xerces-c_3D PATHS "${PROJECT_SOURCE_DIR}/../xercescpp" "${PROJECT_SOURCE_DIR}/../lib/xercescpp" "$ENV{CMAKE_HINT}/xercescpp" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_path(XercescppLib_include_DIR NAMES xercesc/dom/DOM.hpp PATHS "${PROJECT_SOURCE_DIR}/../xercescpp" "${PROJECT_SOURCE_DIR}/../lib/xercescpp" "$ENV{CMAKE_HINT}/xercescpp" ENV CMAKE_HINT PATH_SUFFIXES "include")
find_path(LibXSD_root_DIR NAMES xsd/cxx/tree/parsing/int.hxx PATHS "${PROJECT_SOURCE_DIR}/../xsd" "${PROJECT_SOURCE_DIR}/../lib/xsd" "$ENV{CMAKE_HINT}/xsd" ENV CMAKE_HINT PATH_SUFFIXES "libxsd")
find_package(regxmllibc)

if(ARCHIVIST)
find_library(OpenEXRLib_IlmImf_PATH NAMES IlmImf IlmImf-2_2 PATHS "${PROJECT_SOURCE_DIR}/../openexr" "${PROJECT_SOURCE_DIR}/../lib/openexr" "$ENV{CMAKE_HINT}/openexr" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(OpenEXRLib_IlmImf_Debug_PATH NAMES IlmImf IlmImf-2_2 PATHS "${PROJECT_SOURCE_DIR}/../openexr" "${PROJECT_SOURCE_DIR}/../lib/openexr" "$ENV{CMAKE_HINT}/openexr" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_path(OpenEXRLib_include_DIR NAMES OpenEXR/ImfAttribute.h PATHS "${PROJECT_SOURCE_DIR}/../openexr" "${PROJECT_SOURCE_DIR}/../lib/openexr" "$ENV{CMAKE_HINT}/openexr" ENV CMAKE_HINT PATH_SUFFIXES "include")
find_library(IlmBaseLib_Imath_PATH NAMES Imath Imath-2_2 PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_Imath_Debug_PATH NAMES Imath Imath-2_2 PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_IlmThread_PATH NAMES IlmThread IlmThread-2_2 PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_IlmThread_Debug_PATH NAMES IlmThread IlmThread-2_2 PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_Iex_PATH NAMES Iex Iex-2_2 PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_Iex_Debug_PATH NAMES Iex Iex-2_2 PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_Half_PATH NAMES Half PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(IlmBaseLib_Half_Debug_PATH NAMES Half PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_path(IlmBaseLib_include_DIR NAMES OpenEXR/half.h PATHS "${PROJECT_SOURCE_DIR}/../ilmbase" "${PROJECT_SOURCE_DIR}/../lib/ilmbase" "$ENV{CMAKE_HINT}/ilmbase" ENV CMAKE_HINT PATH_SUFFIXES "include")
find_library(ZLib_PATH NAMES z zlib PATHS "${PROJECT_SOURCE_DIR}/../zlib" "${PROJECT_SOURCE_DIR}/../lib/zlib" "$ENV{CMAKE_HINT}/zlib" ENV CMAKE_HINT PATH_SUFFIXES "lib")
find_library(ZLib_Debug_PATH NAMES z zlibd PATHS "${PROJECT_SOURCE_DIR}/../zlib" "${PROJECT_SOURCE_DIR}/../lib/zlib" "$ENV{CMAKE_HINT}/zlib" ENV CMAKE_HINT PATH_SUFFIXES "lib")
endif(ARCHIVIST)

include("${asdcplib_Targets_PATH}") # Import asdcplib targets.
include("${OpenJPEG_Targets_PATH}") # Import OpenJPEG targets

#find_package(OpenCL)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Multimedia)
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
	message(STATUS "Use target \"doc\" to generate doxygen.")
endif(DOXYGEN_FOUND)

# Find includes in corresponding build directories.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(qt_rcc_resources "${PROJECT_SOURCE_DIR}/resources/qt_resources.qrc")
set(win_resources "${PROJECT_SOURCE_DIR}/resources/win_resources.rc")

# source
set(tool_src main.cpp MainWindow.cpp KMQtLogSink.cpp QtWaitingSpinner.cpp WidgetAbout.cpp
	WidgetComposition.cpp ImfCommon.cpp WidgetImpBrowser.cpp ImfPackage.cpp ImfPackageCommon.cpp WizardWorkspaceLauncher.cpp 
	ImfPackageCommands.cpp MetadataExtractor.cpp MetadataExtractorCommon.cpp WizardResourceGenerator.cpp DelegateComboBox.cpp DelegateMetadata.cpp
	GraphicsWidgetTimeline.cpp GraphicsWidgetSegment.cpp CompositionPlaylistCommands.cpp ImfMimeData.cpp GraphicsCommon.cpp
	CustomProxyStyle.cpp GraphicScenes.cpp GraphicsWidgetResources.cpp GraphicsViewScaleable.cpp WidgetTrackDedails.cpp GraphicsWidgetComposition.cpp
	GraphicsWidgetSequence.cpp Events.cpp WidgetCentral.cpp
	WidgetCompositionInfo.cpp UndoProxyModel.cpp JobQueue.cpp Jobs.cpp Error.cpp EmptyTimedTextGenerator.cpp WizardPartialImpGenerator.cpp
	WidgetVideoPreview.cpp WidgetImagePreview.cpp JP2K_Preview.cpp JP2K_Player.cpp JP2K_Decoder.cpp TTMLParser.cpp WidgetTimedTextPreview.cpp TimelineParser.cpp createLUTs.cpp # (k)
	WidgetContentVersionList.cpp WidgetContentVersionListCommands.cpp WidgetLocaleList.cpp WidgetLocaleListCommands.cpp#WR
	)

# header
set(tool_src ${tool_src} global.h MainWindow.h KMQtLogSink.h QtWaitingSpinner.h 
	WidgetAbout.h WidgetComposition.h ImfCommon.h WidgetImpBrowser.h ImfPackage.h ImfPackageCommon.h WizardWorkspaceLauncher.h 
	ImfPackageCommands.h MetadataExtractor.h MetadataExtractorCommon.h WizardResourceGenerator.h DelegateComboBox.h DelegateMetadata.h 
	GraphicsWidgetTimeline.h GraphicsWidgetSegment.h CompositionPlaylistCommands.h ImfMimeData.h GraphicsCommon.h
	CustomProxyStyle.h GraphicScenes.h GraphicsWidgetResources.h GraphicsViewScaleable.h WidgetTrackDedails.h GraphicsWidgetComposition.h
	GraphicsWidgetSequence.h Events.h WidgetCentral.h Int24.h
	WidgetCompositionInfo.h UndoProxyModel.h SafeBool.h JobQueue.h Jobs.h Error.h EmptyTimedTextGenerator.h WizardPartialImpGenerator.h
	WidgetVideoPreview.h WidgetImagePreview.h JP2K_Preview.h JP2K_Player.h JP2K_Decoder.h TTMLParser.h WidgetTimedTextPreview.h TimelineParser.h createLUTs.h SMPTE_Labels.h # (k)
	WidgetContentVersionList.h WidgetContentVersionListCommands.h WidgetLocaleList.h WidgetLocaleListCommands.h# WR
	)

set(synthesis_src synthesis/dcmlTypes.cpp synthesis/SMPTE-429-8-2006-PKL.cpp synthesis/SMPTE-429-8-2014-AM.cpp synthesis/SMPTE-2067-2-2013-Core.cpp 
	synthesis/SMPTE-2067-3-2013-CPL.cpp synthesis/SMPTE-2067-100a-2014-OPL.cpp synthesis/xml.cpp synthesis/xmldsig-core-schema.cpp
	synthesis/st2067-2a-2016-Core.cpp synthesis/st2067-2b-2016-PKL.cpp synthesis/st2067-3a-2016-CPL.cpp
	)

source_group("Resource Files" FILES ${qt_rcc_resources} ${win_resources} ${open_cl_src})
source_group("Synthesis Files" FILES ${synthesis_src})
qt5_add_resources(resSources ${qt_rcc_resources})

if(ARCHIVIST)
include_directories("${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/synthesis" "${LibXSD_root_DIR}" "${XercescppLib_include_DIR}" 
	"${asdcplib_include_DIR}" "${OpenEXRLib_include_DIR}/OpenEXR" "${IlmBaseLib_include_DIR}/OpenEXR")
else(ARCHIVIST)
include_directories("${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/synthesis" "${LibXSD_root_DIR}" "${XercescppLib_include_DIR}" 
	"${asdcplib_include_DIR}" "${OpenJPEG_include_DIR}" "${REGXMLLIBC_INCLUDE_DIRS}")
endif(ARCHIVIST)

add_definitions(/DLIBAS02MOD)

if(WIN32)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS /DUNICODE /DKM_WIN32 /DASDCP_PLATFORM=\"win32\" /DNOMINMAX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
elseif(UNIX)
	add_definitions(/DASDCP_PLATFORM=\"unix\")
endif(WIN32)

add_executable(${EXE_NAME} WIN32 ${tool_src} ${resSources} ${win_resources} ${synthesis_src})
if(ARCHIVIST)
target_link_libraries(${EXE_NAME} general Qt5::Widgets general Qt5::Multimedia debug "${ZLib_Debug_PATH}" optimized "${ZLib_PATH}" debug "${IlmBaseLib_Half_Debug_PATH}" optimized "${IlmBaseLib_Half_PATH}" debug "${IlmBaseLib_IlmThread_Debug_PATH}" optimized "${IlmBaseLib_IlmThread_PATH}" debug "${IlmBaseLib_Iex_Debug_PATH}" optimized "${IlmBaseLib_Iex_PATH}"
	 debug "${IlmBaseLib_Imath_Debug_PATH}" optimized "${IlmBaseLib_Imath_PATH}" debug "${OpenEXRLib_IlmImf_Debug_PATH}" optimized "${OpenEXRLib_IlmImf_PATH}" general libas02 debug "${XercescppLib_Debug_PATH}" optimized "${XercescppLib_PATH}")
else(ARCHIVIST)
target_link_libraries(${EXE_NAME} general Qt5::Widgets general Qt5::Multimedia 
	 general libas02 debug "${XercescppLib_Debug_PATH}" optimized "${XercescppLib_PATH}" debug "${OpenJPEGLib_Debug_Path}" optimized "${OpenJPEGLib_Path}"
	 debug regxmllibc_d optimized regxmllibc
	 )
endif(ARCHIVIST)

# add the install target
install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib)
