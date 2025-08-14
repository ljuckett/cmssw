import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Run3_2025_cff import Run3_2025
process = cms.Process('PIXELDQMDEV',Run3_2025)

#from Configuration.Eras.Era_Run2_2017_cff import Run2_2017
#process = cms.Process('PIXELDQMDEV',Run2_2017)

from Configuration.Eras.Era_Phase2C11M9_cff import Phase2C11M9

#process = cms.Process('PIXELDQMDEV',Phase2C11M9)


# Input source
# dataset /RelValMinBias_13/CMSSW_8_1_0_pre16-81X_upgrade2017_realistic_v22-v1/GEN-SIM-DIGI-RAW

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/eos/cms/store/data/Run2025C/ZeroBias/RAW/v1/000/392/477/00000/0c340d06-5e94-4c63-a005-41d640e78f1f.root'),
    #fileNames = cms.untracked.vstring('/store/relval/CMSSW_14_0_0_pre2/RelValTTbar_14TeV/GEN-SIM-DIGI-RAW/PU_133X_mcRun4_realistic_v1_STD_2026D98_PU200_RV229-v1/2580000/0b2b0b0b-f312-48a8-9d46-ccbadc69bbfd.root'),
    secondaryFileNames = cms.untracked.vstring()
)

process.load('Configuration.EventContent.EventContent_cff')

process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('step1_RAW2DIGI_L1Reco_RECO_DQM.root'),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(50),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)


#----------------------------
# DQM Environment
#-----------------------------
process.load("DQMServices.Components.DQMEnvironment_cfi")

#----------------------------
# DQM Live Environment
#-----------------------------
dqmRunConfigDefaults = {
    'userarea': cms.PSet(
        type = cms.untracked.string("userarea"),
        collectorPort = cms.untracked.int32(9190),
        collectorHost = cms.untracked.string('lxplus064'),
    ),
}

dqmRunConfigType = "userarea"
dqmRunConfig = dqmRunConfigDefaults[dqmRunConfigType]

process.load("DQMServices.Core.DQMStore_cfi")

process.DQM = cms.Service("DQM",
                  debug = cms.untracked.bool(False),
                  publishFrequency = cms.untracked.double(5.0),
                  collectorPort = dqmRunConfig.collectorPort,
                  collectorHost = dqmRunConfig.collectorHost,
                  filter = cms.untracked.string(''),
)

process.DQMMonitoringService = cms.Service("DQMMonitoringService")

process.load("DQMServices.Components.DQMEventInfo_cfi")
#process.load("DQMServices.FileIO.DQMFileSaverOnline_cfi")

# upload should be either a directory or a symlink for dqm gui destination
#process.dqmSaver.path = "." 
#process.dqmSaver.producer = 'DQM'
#process.dqmSaver.backupLumiCount = 15

TAG = "PixelPhase1"
process.dqmEnv.subSystemFolder = TAG
#process.dqmSaver.tag = TAG

#-----------------------------
# Magnetic Field
#-----------------------------
# 3.8T field
process.load('Configuration.StandardSequences.MagneticField_cff')

#-------------------------------------------------
# GEOMETRY
#-------------------------------------------------
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
#process.load('Configuration.Geometry.GeometryExtendedRun4D98Reco_cff')

#-------------------------------------------------
# GLOBALTAG
#-------------------------------------------------
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '150X_dataRun3_Prompt_v1', '')


#-----------------------
#  Reconstruction Modules
#-----------------------
# Real data raw to digi
process.load("Configuration.StandardSequences.RawToDigi_Data_cff")
process.load("EventFilter.SiPixelRawToDigi.SiPixelRawToDigi_cfi")
process.siPixelDigis.IncludeErrors = True

# Local Reconstruction
process.load("RecoLocalTracker.SiPixelClusterizer.SiPixelClusterizer_cfi")

process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("Configuration.StandardSequences.RawToDigi_cff")
process.load("Configuration.StandardSequences.L1Reco_cff") 

#-----------------------
#  Phase1 DQM
#-----------------------



# first, we load the global  defaults and overwrite what needs to be changed
from DQM.SiPixelPhase1Common.HistogramManager_cfi import *
DefaultHisto.enabled = True
DefaultHisto.topFolderName = TAG 

# maximum Lumisection number for trends. This is a hard limit, higher ends up in overflow.
SiPixelPhase1Geometry.max_lumisection = 1000 
# #LS per line in the "overlaid curves"
SiPixelPhase1Geometry.onlineblock = 10 
# number of lines
SiPixelPhase1Geometry.n_onlineblocks = int(SiPixelPhase1Geometry.max_lumisection.value()/SiPixelPhase1Geometry.onlineblock.value())

# then, we load the online config. This will overwrite more defaults, and e.g. configure for phase0 real data.
#process.load("DQM.SiPixelPhase1Config.SiPixelPhase1OnlineDQM_cff")
process.load("DQM.SiPixelPhase1Config.SiPixelPhase1OfflineDQM_harvesting_cff")
#process.load("RecoLocalTracker.SiPixelClusterizer.SiPixelClusterizer_cfi")
# this also loads the plugins. After that, some values cannot be changed any more, since they were copied around.

# Now change things back to Phase1 MC
SiPixelPhase1Geometry.upgradePhase = 1


#process.load('RecoTracker.Configuration.RecoTracker_cff')
#    
#    #process.newCombinedSeeds.seedCollections = cms.VInputTag(
#    #    cms.InputTag('initialStepSeeds'),
#    #    )
#    
#process.load('RecoTracker.FinalTrackSelectors.MergeTrackCollections_cff')
#
##import RecoTracker.FinalTrackSelectors.earlyGeneralTracks_cfi
#
#process.load('RecoTracker.FinalTrackSelectors.earlyGeneralTracks_cfi')
#
#
#
#process.earlyGeneralTracks.hasSelector=cms.vint32(1)
#process.earlyGeneralTracks.selectedTrackQuals = cms.VInputTag(
#    #        cms.InputTag("initialStepSelector","initialStep"),
#    cms.InputTag("initialStep"),
#    )
#process.earlyGeneralTracks.setsToMerge = cms.VPSet( cms.PSet( tLists=cms.vint32(0), pQual=cms.bool(True) ) )
#
#process.load("RecoTracker.IterativeTracking.iterativeTk_cff")
#
#process.iterTracking_FirstStep =cms.Sequence(
#    process.InitialStep
#    *process.earlyGeneralTracks
#    )
#
#
##process.earlyGeneralTracks.TrackProducers = (
##    cms.InputTag('initialStepTracks'),
##    )
#
#process.RecoForDQM_LocalReco = cms.Sequence(process.siPixelDigis*process.siStripDigis*process.gtDigis*process.trackerlocalreco)#*process.gtEvmDigis)
#

#from DQM.SiPixelPhase1TrackResiduals.SiPixelPhase1TrackResiduals_cfi import *
# Clusters ontrack/offtrack (also general tracks)
#from DQM.SiPixelPhase1TrackClusters.SiPixelPhase1TrackClusters_cfi import *
# Hit Efficiencies
#from DQM.SiPixelPhase1TrackEfficiency.SiPixelPhase1TrackEfficiency_cfi import *

process.siPixelDigis.InputLabel = cms.InputTag("rawDataCollector")
process.SiPixelPhase1DigisAnalyzer.src = "siPixelDigis"
process.SiPixelPhase1RawDataAnalyzer.src = "siPixelDigis"

#process.RecoForDQM_TrkReco = cms.Sequence(process.offlineBeamSpot*process.MeasurementTrackerEventPreSplitting*process.siPixelClusterShapeCachePreSplitting*process.recopixelvertexing*process.InitialStepPreSplitting)
# All plot configurations should go the the specific config files (for online and offline)
# or to SiPixelPhase1OnlineDQM_cff (if online only). Refer to pixel_up_dqm_sourceclient-live_cfg.py
# to see how things could be overwritten here (works the same in SiPixelPhase1OnlineDQM_cff).

#--------------------------
# Service
#--------------------------
process.AdaptorConfig = cms.Service("AdaptorConfig")

#--------------------------
# Scheduling
#--------------------------
process.raw2digi_step = cms.Path(process.RawToDigi)
#process.raw2digi_step = cms.Path(process.siPixelDigis)
process.l1reco_step = cms.Path(process.L1Reco)
process.reconstruction_step = cms.Path(process.reconstruction)
process.DQMmodules = cms.Path(process.dqmEnv)#*process.dqmSaver)
process.offlineDQM_step = cms.Path(process.siPixelPhase1OfflineDQM_source)
process.offlineHarvesting_step = cms.Path(process.siPixelPhase1OfflineDQM_harvesting)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)

process.schedule = cms.Schedule(
  process.raw2digi_step, 
  process.l1reco_step, 
  process.reconstruction_step,
  #process.siPixelDigis,
  #* process.siPixelClusters
  process.DQMmodules,
  process.offlineDQM_step,
  process.offlineHarvesting_step,
  #* process.SiPixelPhase1GeometryDebugAnalyzer
  #* process.SiPixelPhase1GeometryDebugHarvester
  process.DQMoutput_step
  )
    
