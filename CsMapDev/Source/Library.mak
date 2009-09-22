
C_FLG = -c -DGCC_3 -D__CPP__ -Wall -O3 -I../Include

CPP_FLG = -c -DGCC_3 -D__CPP__ -Wall -O3 -I../Include

.cpp.o:
	gcc $(CPP_FLG) $<

.c.o:
	gcc $(C_FLG) $<

CSMAP_LIB_SRC = \
CS_agd66.c \
CS_agd84.c \
CS_alber.c \
CS_angle.c \
CS_ansi.c \
CS_ats77.c \
CS_ats77Transform.c \
CS_azmea.c \
CS_azmed.c \
CS_bonne.c \
CS_bpcnc.c \
CS_bynFile.c \
CS_caDatum.c \
CS_category.c \
CS_caV1GridFile.c \
CS_caV2GridFile.c \
CScs2Wkt.c \
cscscomp.c \
CS_ch1903.c \
CS_csini.c \
CS_csio.c \
CS_csprm.c \
CS_csrs.c \
csCsvFileSupport.cpp \
CS_csWktLoc.c \
CSdata.c \
CSdataPJ.c \
CSdataU.c \
CS_datum.c \
CSdatumCatalog.c \
CSdatumSupport.c \
CS_defaults.c \
CS_defCmp.c \
CSdictDiff.c \
CSdt2Wkt.c \
CS_dtAgd66ToGda94.c \
CS_dtAgd84ToGda94.c \
CS_dtAts77ToNad83.c \
CS_dtCh1903ToPlus.c \
CS_dtcalc.c \
CSdtcomp.c \
CS_dtEd50ToEtrf89.c \
CS_dtDhdnToEtrf89.c \
CS_dtio.c \
CS_dtTokyoToJgd2k.c \
CS_dtNad27ToAts77.c \
CS_dtNad27ToNad83.c \
CS_dtNad83ToHarn.c \
CS_dtNzdg49ToNzgd2K.c \
CS_dtRgf93ToNtf.c \
CS_dhdn.c \
CS_ed50.c \
CS_edcnc.c \
CS_edcyl.c \
CS_egm96.c \
CS_ekrt4.c \
CS_ekrt6.c \
CSel2Wkt.c \
CS_elCalc.c \
CSelcomp.c \
CS_elio.c \
CS_erpt.c \
CS_error.c \
CS_fips.c \
CS_gauss.c \
CS_general.c \
CS_geoid96.c \
CS_geoid99.c \
CS_GeoidHeight.c \
CS_gissupprt.c \
CS_gnomc.c \
CSgridCellCache.c \
CS_groups.c \
CS_guiApi.c \
CS_hlApi.c \
CS_hmlsn.c \
CS_hpApi.c \
CS_hpgn.c \
CS_japan.c \
CS_krovk.c \
CS_lmbrt.c \
CS_lmtan.c \
CS_mfc.cpp \
CS_mgrs.c \
CS_millr.c \
CS_modpc.c \
CS_molwd.c \
CS_mrcat.c \
CSmrcomp.c \
CS_mstro.c \
CS_nacyl.c \
CS_nad27.c \
CSnad27ToCsrs.c \
CSnad83ToCsrs.c \
csNameMapper.cpp \
csNameMapperSupport.cpp \
CS_nerth.c \
CS_nzgd49.c \
CS_nzlnd.c \
CS_oblqm.c \
CS_optional.c \
CS_ortho.c \
CS_osgm91.c \
CS_ostn02.c \
CS_ostn97.c \
CS_ostro.c \
CS_plycn.c \
CS_pstro.c \
CS_rgf93ToNtf.c \
CS_rlsUpdt.c \
CS_robin.c \
CS_sinus.c \
CS_sstro.c \
CS_supprt.c \
CS_swiss.c \
CS_sys34.c \
CS_system.c \
CS_tacyl.c \
CS_trmer.c \
CS_trmrs.c \
CS_units.c \
CS_unity.c \
CS_usDatum.c \
CS_usGridFile.c \
CS_vdgrn.c \
CS_VertconUS.c \
CS_vrtcon.c \
CS_winkelTripel.c \
CSwktFlavors.c \
CS_zones.c \
cs_wellknowntext.cpp \
rcWellKnownText.cpp \
rcWktKonstants.cpp


CSMAP_LIB_OBJ = \
CS_agd66.o \
CS_agd84.o \
CS_alber.o \
CS_angle.o \
CS_ansi.o \
CS_ats77.o \
CS_ats77Transform.o \
CS_azmea.o \
CS_azmed.o \
CS_bonne.o \
CS_bpcnc.o \
CS_bynFile.o \
CS_caDatum.o \
CS_category.o \
CS_caV1GridFile.o \
CS_caV2GridFile.o \
CS_ch1903.o \
CScs2Wkt.o \
cscscomp.o \
CS_csini.o \
CS_csio.o \
CS_csprm.o \
CS_csrs.o \
csCsvFileSupport.o \
CS_csWktLoc.o \
CSdata.o \
CSdataPJ.o \
CSdataU.o \
CS_datum.o \
CSdatumCatalog.o \
CSdatumSupport.o \
CS_defaults.o \
CS_defCmp.o \
CSdictDiff.o \
CSdt2Wkt.o \
CS_dtAgd66ToGda94.o \
CS_dtAgd84ToGda94.o \
CS_dtAts77ToNad83.o \
CS_dtcalc.o \
CSdtcomp.o \
CS_dtCh1903ToPlus.o \
CS_dtDhdnToEtrf89.o \
CS_dtEd50ToEtrf89.o \
CS_dtio.o \
CS_dtTokyoToJgd2k.o \
CS_dtNad27ToAts77.o \
CS_dtNad27ToNad83.o \
CS_dtNad83ToHarn.o \
CS_dtNzdg49ToNzgd2K.o \
CS_dtRgf93ToNtf.o \
CS_ed50.o \
CS_dhdn.o \
CS_edcnc.o \
CS_edcyl.o \
CS_egm96.o \
CS_ekrt4.o \
CS_ekrt6.o \
CSel2Wkt.o \
CS_elCalc.o \
CSelcomp.o \
CS_elio.o \
CS_erpt.o \
CS_error.o \
CS_fips.o \
CS_gauss.o \
CS_general.o \
CS_geoid96.o \
CS_geoid99.o \
CS_GeoidHeight.o \
CS_gissupprt.o \
CS_gnomc.o \
CSgridCellCache.o \
CS_groups.o \
CS_guiApi.o \
CS_hlApi.o \
CS_hmlsn.o \
CS_hpApi.o \
CS_hpgn.o \
CS_japan.o \
CS_krovk.o \
CS_lmbrt.o \
CS_lmtan.o \
CS_mfc.o \
CS_mgrs.o \
CS_millr.o \
CS_modpc.o \
CS_molwd.o \
CS_mrcat.o \
CSmrcomp.o \
CS_mstro.o \
CS_nacyl.o \
CS_nad27.o \
CSnad27ToCsrs.o \
CSnad83ToCsrs.o \
csNameMapper.o \
csNameMapperSupport.o \
CS_nerth.o \
CS_nzgd49.o \
CS_nzlnd.o \
CS_oblqm.o \
CS_optional.o \
CS_ortho.o \
CS_osgm91.o \
CS_ostn02.o \
CS_ostn97.o \
CS_ostro.o \
CS_plycn.o \
CS_pstro.o \
CS_rgf93ToNtf.o \
CS_rlsUpdt.o \
CS_robin.o \
CS_sinus.o \
CS_sstro.o \
CS_supprt.o \
CS_swiss.o \
CS_sys34.o \
CS_system.o \
CS_tacyl.o \
CS_trmer.o \
CS_trmrs.o \
CS_units.o \
CS_unity.o \
CS_usDatum.o \
CS_usGridFile.o \
CS_vdgrn.o \
CS_VertconUS.o \
CS_vrtcon.o \
CS_winkelTripel.o \
CSwktFlavors.o \
CS_zones.o \
cs_wellknowntext.o \
rcWellKnownText.o \
rcWktKonstants.o

CsMap.a : $(CSMAP_LIB_OBJ)
	ar rv CsMap.a $?

