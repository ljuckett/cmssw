import FWCore.ParameterSet.Config as cms


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
