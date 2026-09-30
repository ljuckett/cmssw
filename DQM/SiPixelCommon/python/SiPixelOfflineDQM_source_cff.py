import FWCore.ParameterSet.Config as cms

# Pixel Track Monitoring
from DQM.SiPixelMonitorTrack.RefitterForPixelDQM import *
from DQM.SiPixelMonitorTrack.SiPixelMonitorTrack_cfi import *
SiPixelTrackResidualSource.saveFile = False
from DQM.SiPixelMonitorTrack.SiPixelMonitorTrack_Cosmics_cfi import *
SiPixelTrackResidualSource_Cosmics.saveFile = False
from DQM.SiPixelMonitorTrack.SiPixelMonitorEfficiency_cfi import *
SiPixelHitEfficiencySource.saveFile = False
from DQM.TrackerMonitorTrack.SiPixelMonitorTrackResiduals_cfi import *

#Track
SiPixelTrackResidualSource.modOn = False
SiPixelTrackResidualSource.ladOn = True
SiPixelTrackResidualSource.layOn = True
SiPixelTrackResidualSource.phiOn = False	
SiPixelTrackResidualSource.bladeOn = True
SiPixelTrackResidualSource.diskOn = True
SiPixelTrackResidualSource.ringOn = False
SiPixelTrackResidualSource_Cosmics.modOn = False
SiPixelTrackResidualSource_Cosmics.ladOn = True
SiPixelTrackResidualSource_Cosmics.layOn = True
SiPixelTrackResidualSource_Cosmics.phiOn = False	
SiPixelTrackResidualSource_Cosmics.bladeOn = True
SiPixelTrackResidualSource_Cosmics.diskOn = True
SiPixelTrackResidualSource_Cosmics.ringOn = False
SiPixelHitEfficiencySource.modOn = False
SiPixelHitEfficiencySource.ladOn = True
SiPixelHitEfficiencySource.layOn = False
SiPixelHitEfficiencySource.phiOn = False
SiPixelHitEfficiencySource.bladeOn = True
SiPixelHitEfficiencySource.diskOn = False
SiPixelHitEfficiencySource.ringOn = False

#HI track modules
hiTracks = "hiGeneralTracks"
hiRefittedForPixelDQM= refittedForPixelDQM.clone(
    src = hiTracks
)

SiPixelTrackResidualSource_HeavyIons = SiPixelTrackResidualSource.clone(
    vtxsrc = 'hiSelectedVertex'
)

SiPixelHitEfficiencySource_HeavyIons = SiPixelHitEfficiencySource.clone(
    vtxsrc = 'hiSelectedVertex'
)


#DQM service
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
dqmInfo = DQMEDAnalyzer('DQMEventInfo',
    subSystemFolder = cms.untracked.string('Pixel')
)

siPixelOfflineDQM_source = cms.Sequence(dqmInfo)

siPixelOfflineDQM_cosmics_source = cms.Sequence(dqmInfo)

siPixelOfflineDQM_heavyions_source = cms.Sequence(dqmInfo)

siPixelOfflineDQM_source_woTrack = cms.Sequence(dqmInfo)

# Phase1 config
# _all_ of the stuff above becomes obsolete. We just hijack the names and 
# replace them with the phase1 config of the new DQM.
from DQM.SiPixelPhase1Config.SiPixelPhase1OfflineDQM_source_cff import *
from Configuration.Eras.Modifier_phase1Pixel_cff import phase1Pixel
phase1Pixel.toReplaceWith(siPixelOfflineDQM_source, siPixelPhase1OfflineDQM_source)
phase1Pixel.toReplaceWith(siPixelOfflineDQM_cosmics_source, siPixelPhase1OfflineDQM_source_cosmics)
phase1Pixel.toReplaceWith(siPixelOfflineDQM_heavyions_source, siPixelPhase1OfflineDQM_source_hi)
# don't forget the Harvesters, they are plugged in at PixelOfflineDQMClient
# TODO: the same game for the other three.
