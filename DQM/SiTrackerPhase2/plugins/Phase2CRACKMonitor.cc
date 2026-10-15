// -*- C++ -*-
//
// Package:    SiOuterTracker
// Class:      SiOuterTracker
//
/**\class SiOuterTracker Phase2OTMonitorTTStub.cc
 DQM/SiOuterTracker/plugins/Phase2OTMonitorTTStub.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Isis Marina Van Parijs
//         Created:  Fri, 24 Oct 2014 12:31:31 GMT
// Edited: August 2026 by Lisa Juckett for dqm output restructure
//

// system include files
#include <memory>
#include <numeric>
#include <vector>

// user include files
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "DataFormats/L1TrackTrigger/interface/TTStub.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/Phase2TrackerCluster/interface/Phase2TrackerCluster1D.h"

#include "Geometry/CommonTopologies/interface/GeomDet.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQM/SiTrackerPhase2/interface/TrackerPhase2DQMUtil.h"
#include "L1Trigger/TrackFindingTracklet/interface/Settings.h"
#include "SimDataFormats/Associations/interface/TTStubAssociationMap.h"
#include "SimDataFormats/Associations/interface/TTTrackAssociationMap.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"

class Phase2CRACKMonitor : public DQMEDAnalyzer {
public:
  explicit Phase2CRACKMonitor(const edm::ParameterSet &);
  ~Phase2CRACKMonitor() override;
  void analyze(const edm::Event &, const edm::EventSetup &) override;
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void dqmBeginRun(const edm::Run &iRun, const edm::EventSetup &iSetup) override;
  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);
  // TTStub stacks
  // Global position of the stubs
  MonitorElement *Stub_Barrel_XY = nullptr;     // TTStub barrel y vs x
  MonitorElement *Stub_RZ = nullptr;            // TTStub #rho vs. z
  MonitorElement *StubCrackOverview = nullptr;      // Cosmic rack: TTStub layer vs module
  MonitorElement* ClusterCrackOverview = nullptr;
  // Number of stubs
  MonitorElement *Stub_Barrel = nullptr;  // TTStub per layer

  // Stub distribution
  MonitorElement *Stub_Eta = nullptr;     // TTstub eta distribution
  MonitorElement *Stub_Phi = nullptr;     // TTstub phi distribution
  MonitorElement *Stub_R = nullptr;       // TTstub r distribution
  MonitorElement *Stub_bendFE = nullptr;  // TTstub trigger bend
  MonitorElement *Stub_bendBE = nullptr;  // TTstub hardware bend
  MonitorElement *Stub_isPS = nullptr;    // is this stub a PS module?

  // STUB Displacement - offset
  MonitorElement *Stub_Barrel_W = nullptr;  // TTstub Pos-Corr Displacement (layer)
  MonitorElement *Stub_Barrel_O = nullptr;  // TTStub Offset (layer)

    MonitorElement* clusters_globalXY_S_;
  MonitorElement* clusters_globalRZ_S_;
  MonitorElement* numberClusters_Barrel_;

  MonitorElement* noisePerEvent = nullptr;
  std::vector<int> layerCheck = {0, 0, 0, 0, 0, 0};

private:
  struct TTStubMEs {
    MonitorElement *NStubs = nullptr;
    unsigned int stubCounter = 0;
    //MonitorElement *StubVisualiser = nullptr; // WHOLE_TK
  };
  std::map<int, TTStubMEs> stubLayerMEs_;
    
  struct ClusterMEs {
    MonitorElement* nClusters_S = nullptr;
    MonitorElement* ClusterSize_S = nullptr;
    std::vector<MonitorElement*> PositionOfClusters_2S;        // CRACK ONLY
    std::vector<MonitorElement*> PositionOfClusters_2SLadder;  // Barrel and LAYER only
    unsigned int clusterCounterS = 0;
  };
  std::map<int, ClusterMEs> clusterLayerMEs_;

  void bookLayerHistos(DQMStore::IBooker &ibooker, uint32_t det_id, std::string &subdir);
  edm::ParameterSet conf_;
  edm::EDGetTokenT<edmNew::DetSetVector<TTStub<Ref_Phase2TrackerDigi_>>> tagTTStubsToken_;
    edm::EDGetTokenT<TTStubAssociationMap<Ref_Phase2TrackerDigi_>>
      ttStubMCTruthToken_;  // MC truth association map for stubs
  std::string topFolderName_;
  edm::EDGetTokenT<Phase2TrackerCluster1DCollectionNew> clustersToken_;
  const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomToken_;
  const edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topoToken_;
  const TrackerGeometry *tkGeom_ = nullptr;
  const TrackerTopology *tTopo_ = nullptr;
};

// constructors and destructor
Phase2CRACKMonitor::Phase2CRACKMonitor(const edm::ParameterSet &iConfig)
    : conf_(iConfig),
	clustersToken_(consumes<Phase2TrackerCluster1DCollectionNew>(conf_.getParameter<edm::InputTag>("clusterSrc"))),
      geomToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()),
      topoToken_(esConsumes<TrackerTopology, TrackerTopologyRcd, edm::Transition::BeginRun>()) {
  // now do what ever initialization is needed
  topFolderName_ = conf_.getParameter<std::string>("TopFolderName");
  tagTTStubsToken_ =
      consumes<edmNew::DetSetVector<TTStub<Ref_Phase2TrackerDigi_>>>(conf_.getParameter<edm::InputTag>("TTStubs"));
    trackingParticleToken_ =
      consumes<std::vector<TrackingParticle>>(conf_.getParameter<edm::InputTag>("trackingParticleToken"));
  ttStubMCTruthToken_ =
      consumes<TTStubAssociationMap<Ref_Phase2TrackerDigi_>>(conf_.getParameter<edm::InputTag>("MCTruthStubInputTag"));
}

Phase2CRACKMonitor::~Phase2CRACKMonitor() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
	//std::cout << eventNumbers << std::endl << std::endl;
}

void Phase2CRACKMonitor::dqmBeginRun(const edm::Run &iRun, const edm::EventSetup &iSetup) {
  tkGeom_ = &(iSetup.getData(geomToken_));
  tTopo_ = &(iSetup.getData(topoToken_));
}
// member functions

// ------------ method called for each event  ------------
void Phase2CRACKMonitor::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) {
  /// Track Trigger Stubs
  edm::Handle<edmNew::DetSetVector<TTStub<Ref_Phase2TrackerDigi_>>> Phase2TrackerDigiTTStubHandle;
  iEvent.getByToken(tagTTStubsToken_, Phase2TrackerDigiTTStubHandle);

  /// Loop over input Stubs
  typename edmNew::DetSetVector<TTStub<Ref_Phase2TrackerDigi_>>::const_iterator inputIter;
  typename edmNew::DetSet<TTStub<Ref_Phase2TrackerDigi_>>::const_iterator contentIter;
    const auto& clusterHandle = iEvent.getHandle(clustersToken_);

      // Truth Association Maps
  edm::Handle<TTStubAssociationMap<Ref_Phase2TrackerDigi_>> MCTruthTTStubHandle;
  iEvent.getByToken(ttStubMCTruthToken_, MCTruthTTStubHandle);
  // Adding protection
  if (!Phase2TrackerDigiTTStubHandle.isValid())
    return;
  //if (Phase2TrackerDigiTTStubHandle->size() < 6)
  //                  return; 

  layerCheck = {0,0,0,0,0,0};
  
  std::cout << Phase2TrackerDigiTTStubHandle->size() << " " << clusterHandle->size() << std::endl;
  // do stubs first
  for (inputIter = Phase2TrackerDigiTTStubHandle->begin(); inputIter != Phase2TrackerDigiTTStubHandle->end();
       ++inputIter) {
    for (contentIter = inputIter->begin(); contentIter != inputIter->end(); ++contentIter) {
      /// Make reference stub
      edm::Ref<edmNew::DetSetVector<TTStub<Ref_Phase2TrackerDigi_>>, TTStub<Ref_Phase2TrackerDigi_>> tempStubRef =
          edmNew::makeRefTo(Phase2TrackerDigiTTStubHandle, contentIter);

      /// Get det ID (place of the stub)
      //  tempStubRef->getDetId() gives the stackDetId, not rawId
      DetId detIdStub = tkGeom_->idToDet((tempStubRef->clusterRef(0))->getDetId())->geographicalId();

      /// Get trigger displacement/offset
      double rawBend = tempStubRef->rawBend();
      double bendOffset = tempStubRef->bendOffset();

      // Get module
      unsigned int module = tTopo_->module(detIdStub);
      int layer = tTopo_->getOTLayerNumber(detIdStub);
      layerCheck[layer-1] = 1;
      // CRACK is viewed from behind, so to align plots with what is seen in real life, modules are flipped
        module = std::abs(int(module - 13));
      /// Define position stub by position inner cluster
      MeasurementPoint mp = (tempStubRef->clusterRef(0))->findAverageLocalCoordinates();
      const GeomDet *theGeomDet = tkGeom_->idToDet(detIdStub);
      Global3DPoint posStub = theGeomDet->surface().toGlobal(theGeomDet->topology().localPosition(mp));

      Stub_Eta->Fill(posStub.eta());
      Stub_Phi->Fill(posStub.phi());
      Stub_R->Fill(posStub.perp());
      Stub_RZ->Fill(posStub.z(), posStub.perp());
      Stub_bendFE->Fill(tempStubRef->bendFE());
      Stub_bendBE->Fill(tempStubRef->bendBE());
      Stub_isPS->Fill(tempStubRef->moduleTypePS());
      
      StubCrackOverview->Fill(module, layer + 0.05 - (module % 2 * 0.1));

      if (detIdStub.subdetId() == static_cast<int>(StripSubdetector::TOB)) {  // Phase 2 Outer Tracker Barrel
        Stub_Barrel->Fill(layer);
        Stub_Barrel_XY->Fill(posStub.x(), posStub.y());
        Stub_Barrel_W->Fill(layer, rawBend - bendOffset);
        Stub_Barrel_O->Fill(layer, bendOffset);
      }

      // Fill layer histograms
      for (int fillingDepth = phase2tkutil::WHOLE_TK; fillingDepth <= phase2tkutil::LAYER; fillingDepth++) {
        // Skip filling for barrel detIds on endcap-only depths
        if ((fillingDepth >= phase2tkutil::SHELL && fillingDepth < phase2tkutil::LAYER) &&
            DetId(detIdStub).subdetId() == SiStripSubdetector::TOB)
          continue;
        int key = phase2tkutil::getNumericHistoId(detIdStub, tTopo_, 0, fillingDepth);
        auto layerMEiter = stubLayerMEs_.find(key);
        if (layerMEiter == stubLayerMEs_.end())
          continue;
        TTStubMEs &local_mes = layerMEiter->second;

        local_mes.stubCounter++;

      }  // end loop fillingDepth
    }  // end loop contentIter
  }  // end loop inputIter
  for (auto &it : stubLayerMEs_) {
    TTStubMEs &local_mes = it.second;
    if (local_mes.NStubs)
      local_mes.NStubs->Fill(local_mes.stubCounter);
    local_mes.stubCounter = 0;
  }

  if (std::find(layerCheck.begin(), layerCheck.end(), 0) == layerCheck.end())
	    noisePerEvent->Fill((clusterHandle->size() - (Phase2TrackerDigiTTStubHandle->size()*2)));

  // CLUSTERS
  if (!clusterHandle.isValid()) {
    edm::LogWarning("Phase2OTMonitorCluster") << "No Phase2TrackerCluster1D Collection found in the event. Skipping!";
    return;
  }
  for (const auto& DSVItr : *clusterHandle) {
    // Getting the id of detector unit
    uint32_t rawid(DSVItr.detId());
    DetId detId(rawid);
    const GeomDetUnit* geomDetUnit(tkGeom_->idToDetUnit(detId));
    if (!geomDetUnit)
      continue;

    TrackerGeometry::ModuleType mType = tkGeom_->getDetectorType(detId);

    for (const auto& clusterItr : DSVItr) {
	    
	    std::cout << "Iterating over clusters!" << std::endl;

      MeasurementPoint mpCluster(clusterItr.center(), clusterItr.column() + 0.5);
      Local3DPoint localPosCluster = geomDetUnit->topology().localPosition(mpCluster);
      Global3DPoint globalPosCluster = geomDetUnit->surface().toGlobal(localPosCluster);
      double gx = globalPosCluster.x();
      double gy = globalPosCluster.y();
      double gz = globalPosCluster.z();
      double gr = globalPosCluster.perp();
      unsigned int module = tTopo_->module(rawid);
      unsigned int ladder = tTopo_->tobRod(rawid);
      int topOrBottomColumn = 0;

      // CRACK is viewed from behind, so to align plots with what is seen in real life, modules are flipped
      module = std::abs(int(module - 13));
      if (mType == TrackerGeometry::ModuleType::Ph2PSS || mType == TrackerGeometry::ModuleType::Ph2SS) {
        clusters_globalXY_S_->Fill(gx, gy);
        clusters_globalRZ_S_->Fill(gz, gr);
      }
      if (detId.subdetId() == SiStripSubdetector::TOB) {
        numberClusters_Barrel_->Fill(tTopo_->layer(detId));
        if (mType == TrackerGeometry::ModuleType::Ph2SS) {
          //If column is on the bottom of the sensor, *-1 to distinguish it from top
          topOrBottomColumn = (tTopo_->isLower(rawid) ? (clusterItr.column() + 1) * -1 : (clusterItr.column() + 1));
	  ClusterCrackOverview->Fill(module, tTopo_->getOTLayerNumber(rawid) + 0.05 - (module % 2 * 0.1));
        }
      }
      std::cout << "begin layer filling!" << std::endl;

      for (int fillingDepth = phase2tkutil::WHOLE_TK; fillingDepth <= phase2tkutil::LAYER; fillingDepth++) {
        // Skip filling for barrel detIds on endcap-only depths
        if ((fillingDepth >= phase2tkutil::SHELL && fillingDepth < phase2tkutil::LAYER) &&
            DetId(detId).subdetId() == SiStripSubdetector::TOB)
          continue;
        int folderkey = phase2tkutil::getNumericHistoId(detId, tTopo_, 0, fillingDepth);

        auto layerMEit = clusterLayerMEs_.find(folderkey);
        if (layerMEit == clusterLayerMEs_.end())
          continue;
        ClusterMEs& local_mes = layerMEit->second;

        if (mType == TrackerGeometry::ModuleType::Ph2PSS || mType == TrackerGeometry::ModuleType::Ph2SS) {
          // Strips
          if (local_mes.ClusterSize_S)
            local_mes.ClusterSize_S->Fill(clusterItr.size());
          local_mes.clusterCounterS++;
          if (mType == TrackerGeometry::ModuleType::Ph2SS) {
            if (!local_mes.PositionOfClusters_2S.empty())
              if (local_mes.PositionOfClusters_2S[module])
                local_mes.PositionOfClusters_2S[module - 1]->Fill(clusterItr.center(), topOrBottomColumn);

            if (!local_mes.PositionOfClusters_2SLadder.empty()) {
              if (local_mes.PositionOfClusters_2SLadder[ladder - 1]) {
                int signedModule = module;
                // CRACK has numbers 1 to 12 while Tracker has 1 to 24
                // Adapt module numbers from 1 to 24 into -12 to +12
                local_mes.PositionOfClusters_2SLadder[ladder - 1]->Fill(signedModule, topOrBottomColumn);
              }
            }
          }
        }
      }
    }
  }
  // After all clusters in event are processed
  for (auto& it : clusterLayerMEs_) {
    ClusterMEs& local_mes = it.second;
    if (local_mes.nClusters_S)
      local_mes.nClusters_S->Fill(local_mes.clusterCounterS);
    local_mes.clusterCounterS = 0;
  }

}  // end of method

void Phase2CRACKMonitor::bookHistograms(DQMStore::IBooker &iBooker, edm::Run const &run, edm::EventSetup const &es) {
  using namespace phase2tkutil;

  // Whole OT Summaries
  iBooker.setCurrentFolder(topFolderName_);

  // CRACK ONLY: module vs layer
  edm::ParameterSet Parameters = conf_.getParameter<edm::ParameterSet>("CrackOverview");
    StubCrackOverview = iBooker.book2DPoly(Parameters.getParameter<std::string>("name"),
                                       Parameters.getParameter<std::string>("title"),
                                       Parameters.getParameter<double>("xmin"),
                                       Parameters.getParameter<double>("xmax"),
                                       Parameters.getParameter<double>("ymin"),
                                       Parameters.getParameter<double>("ymax"));
    if (StubCrackOverview->getTH2Poly()->GetNumberOfBins() == 0) {
      double yOffset = 0;
      for (int layer = 1; layer < 7; layer++) {
        for (int module = 1; module < 13; module++) {
          if (module % 2 == 1)
            yOffset = -0.1;
          else
            yOffset = 0;
          StubCrackOverview->addBin(module - 0.7, layer + yOffset, module + 0.7, layer + yOffset + 0.1);
        }
      }
    }
    StubCrackOverview->getTH2Poly()->SetStats(false);
    StubCrackOverview->setOption("z");

    Parameters = conf_.getParameter<edm::ParameterSet>("ClusterCrackOverview");
    ClusterCrackOverview = iBooker.book2DPoly(Parameters.getParameter<std::string>("name"),
                                       Parameters.getParameter<std::string>("title"),
                                       Parameters.getParameter<double>("xmin"),
                                       Parameters.getParameter<double>("xmax"),
                                       Parameters.getParameter<double>("ymin"),
                                       Parameters.getParameter<double>("ymax"));
    if (ClusterCrackOverview->getTH2Poly()->GetNumberOfBins() == 0) {
      double yOffset = 0;
      for (int layer = 1; layer < 7; layer++) {
        for (int module = 1; module < 13; module++) {
          if (module % 2 == 1)
            yOffset = -0.1;
          else
            yOffset = 0;
          ClusterCrackOverview->addBin(module - 0.7, layer + yOffset, module + 0.7, layer + yOffset + 0.1);
        }
      }
    }
    ClusterCrackOverview->getTH2Poly()->SetStats(false);
    ClusterCrackOverview->setOption("z0");
  // Distributions
  Stub_Eta = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_Eta"), iBooker);
  Stub_Phi = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_Phi"), iBooker);
  Stub_R = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_R"), iBooker);
  Stub_bendFE = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_bendFE"), iBooker);
  Stub_bendBE = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_bendBE"), iBooker);
  Stub_isPS = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_isPS"), iBooker);

  noisePerEvent = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("noise"), iBooker);
  // Positions
  iBooker.setCurrentFolder(topFolderName_ + "/Positions");
  Stub_Barrel_XY = book2DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_Global_Position_Barrel_XY"), iBooker);
  Stub_RZ = book2DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_Global_Position_RZ"), iBooker);
  clusters_globalXY_S_ = phase2tkutil::book2DFromPSet(conf_.getParameter<edm::ParameterSet>("GlobalPositionXY_S"), iBooker);

  clusters_globalRZ_S_ = phase2tkutil::book2DFromPSet(conf_.getParameter<edm::ParameterSet>("GlobalPositionRZ_S"), iBooker);


  // Barrel Summaries
  iBooker.setCurrentFolder(topFolderName_ + "/Barrel");
  Stub_Barrel = book1DFromPSet(conf_.getParameter<edm::ParameterSet>("Num_L1Stubs_Barrel"), iBooker);
  Stub_Barrel_W = book2DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_Width_Barrel"), iBooker);
  Stub_Barrel_O = book2DFromPSet(conf_.getParameter<edm::ParameterSet>("L1Stub_Offset_Barrel"), iBooker);
  numberClusters_Barrel_ = phase2tkutil::book1DFromPSet(conf_.getParameter<edm::ParameterSet>("NClusters_Barrel"), iBooker);

  //Now book layer wise histos
  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;
  if (theTkDigiGeomWatcher.check(es)) {
    for (auto const &det_u : tkGeom_->detUnits()) {
      //Always check TrackerNumberingBuilder before changing this part
      //continue if Pixel
      if ((det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXB ||
           det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXEC))
        continue;
      unsigned int detId_raw = det_u->geographicalId().rawId();
      edm::LogInfo("Phase2OTMonitorTTStub")
          << "Detid:" << detId_raw << "\tsubdet=" << det_u->subDetector()
          << "\t key=" << phase2tkutil::getHistoId(detId_raw, tTopo_, 0.0, phase2tkutil::LAYER, false) << std::endl;
      bookLayerHistos(iBooker, detId_raw, topFolderName_);
    }
  }
}

void Phase2CRACKMonitor::bookLayerHistos(DQMStore::IBooker &ibooker, uint32_t det_id, std::string &subdir) {
  for (int bookingDepth = phase2tkutil::WHOLE_TK; bookingDepth <= phase2tkutil::LAYER; bookingDepth++) {
    // Skip booking at endcap depths for barrel dets
    if ((bookingDepth >= phase2tkutil::SHELL && bookingDepth < phase2tkutil::LAYER) &&
        DetId(det_id).subdetId() == static_cast<int>(StripSubdetector::TOB))
      continue;

    int key = phase2tkutil::getNumericHistoId(det_id, tTopo_, 0.0, bookingDepth);

    if (stubLayerMEs_.find(key) == stubLayerMEs_.end()) {
      std::string folderName = phase2tkutil::getHistoId(det_id, tTopo_, 0.0, bookingDepth, false);
      std::string prettyName = phase2tkutil::getHistoId(det_id, tTopo_, 0.0, bookingDepth, true);
      ibooker.cd();
      ibooker.setCurrentFolder(subdir + "/" + folderName);
      edm::LogInfo("Phase2OTMonitorTTStub") << " Booking Histograms in: " << subdir + "/" + folderName;
      TTStubMEs local_mes;

      local_mes.NStubs = phase2tkutil::book1DFromPSet(
          conf_.getParameter<edm::ParameterSet>("NStubsLayer"), ibooker, prettyName, bookingDepth);

      stubLayerMEs_.emplace(key, local_mes);

      //clusters 

      if (clusterLayerMEs_.find(key) == clusterLayerMEs_.end()) {
      std::string folderName = phase2tkutil::getHistoId(det_id, tTopo_, 0.0, bookingDepth, false);
      std::string prettyName = phase2tkutil::getHistoId(det_id, tTopo_, 0.0, bookingDepth, true);
      ibooker.cd();
      ibooker.setCurrentFolder(subdir + "/" + folderName);
      edm::LogInfo("Phase2OTMonitorCluster") << " Booking Histograms in: " << subdir + "/" + folderName;
      ClusterMEs local_mes;

      TrackerGeometry::ModuleType mType = tkGeom_->getDetectorType(det_id);
      if (mType == TrackerGeometry::ModuleType::Ph2SS && bookingDepth == 6) {
        //Book the right number of histograms per layer
        unsigned int nLadders = 0;
        unsigned int nModules = 0;

        const auto theLayer = tTopo_->getOTLayerNumber(det_id);

        TrackerGeometry::DetIdContainer theDetIds = tkGeom_->detIds();
        for (auto detid : theDetIds) {
          // Only count 2S modules in the same layer as ref
          if (tkGeom_->getDetectorType(detid) != TrackerGeometry::ModuleType::Ph2SS)
            continue;
          if (tTopo_->getOTLayerNumber(detid) != theLayer)
            continue;

          const bool isBarrel = (detid.subdetId() == SiStripSubdetector::TOB);
          if (isBarrel) {
            nLadders = std::max(nLadders, tTopo_->tobRod(detid));
            nModules = std::max(nModules, tTopo_->module(detid));
          }
        }

        //Book the histograms
        local_mes.PositionOfClusters_2SLadder.resize(nLadders, nullptr);
        auto pos2SModulePSet = conf_.getParameter<edm::ParameterSet>("PositionOfClusters_2S");
        if (pos2SModulePSet.getParameter<bool>("switch"))
          local_mes.PositionOfClusters_2S.resize(nModules, nullptr);

        for (unsigned int ladderNum = 1; ladderNum <= nLadders; ladderNum++) {
          auto pos2SLadderPSet = conf_.getParameter<edm::ParameterSet>("PositionOfClusters_2SLadder");
          pos2SLadderPSet.addParameter<std::string>("name", "Position_Of_Clusters_2S_Lad_" + std::to_string(ladderNum));
          pos2SLadderPSet.addParameter<std::string>(
              "title",
              "Position Of Clusters in 2S modules in {} ladder " + std::to_string(ladderNum) + ";Module;Half-module;");
          local_mes.PositionOfClusters_2SLadder[ladderNum - 1] =
              phase2tkutil::book2DFromPSet(pos2SLadderPSet, ibooker, prettyName);
          if (pos2SModulePSet.getParameter<bool>("switch")) {
            for (unsigned int moduleNum = 1; moduleNum <= nModules; moduleNum++) {
              auto pos2SModulePSet = conf_.getParameter<edm::ParameterSet>("PositionOfClusters_2S");
              pos2SModulePSet.addParameter<std::string>(
                  "name",
                  "Position_Of_Clusters_2S_Lad_" + std::to_string(ladderNum) + "_Mod_" + std::to_string(moduleNum));
              pos2SModulePSet.addParameter<std::string>("title",
                                                        "Position Of Clusters in 2S modules in {} ladder " +
                                                            std::to_string(ladderNum) + " module " +
                                                            std::to_string(moduleNum) + ";Strip;Half-module;");
              local_mes.PositionOfClusters_2S[moduleNum - 1] =
                  phase2tkutil::book2DFromPSet(pos2SModulePSet, ibooker, prettyName);
            }
          }
        }
      }

      local_mes.nClusters_S = phase2tkutil::book1DFromPSet(
          conf_.getParameter<edm::ParameterSet>("NClustersLayer_S"), ibooker, prettyName, bookingDepth);

      local_mes.ClusterSize_S =
          phase2tkutil::book1DFromPSet(conf_.getParameter<edm::ParameterSet>("ClusterSize_S"), ibooker, prettyName);

      clusterLayerMEs_.emplace(key, local_mes);
    }  //if block layerME find
    }
  }
}

void Phase2CRACKMonitor::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;

  // CRACK
  phase2tkutil::add2DDesc(desc,
                          "CrackOverview",
                          "Crack_Overview_L1Stubs",
                          "Crack_Overview_stubs",
                          "Module",
                          "Layer",
                          13.0,
                          0.0,
                          13,
                          0.0,
                          0.0,
                          7.5);

  // Position
  phase2tkutil::add2DDesc(desc,
                          "L1Stub_Global_Position_Barrel_XY",
                          "L1Stub_Global_Position_Barrel_XY",
                          "L1Stub_Global_Position_Barrel_XY",
                          "L1 Stub Barrel position x [cm]",
                          "L1 Stub Barrel position y [cm]",
                          960,
                          -120,
                          120,
                          960,
                          -120,
                          120);
  phase2tkutil::add2DDesc(desc,
                          "L1Stub_Global_Position_RZ",
                          "L1Stub_Global_Position_RZ",
                          "L1Stub_Global_Position_RZ",
                          "L1 Stub position z [cm]",
                          "L1 Stub position #rho [cm]",
                          900,
                          -300,
                          300,
                          900,
                          0,
                          120);

  // Stub distributions
  phase2tkutil::add1DDesc(desc, "L1Stub_Eta", "L1Stub_Eta", "L1Stub_Eta", "#eta", "# L1 Stubs", 45, -5, 5);
  phase2tkutil::add1DDesc(desc, "L1Stub_Phi", "L1Stub_Phi", "L1Stub_Phi", "#phi", "# L1 Stubs", 60, -3.5, 3.5);
  phase2tkutil::add1DDesc(desc, "L1Stub_R", "L1Stub_R", "L1Stub_R", "R", "# L1 Stubs", 45, 0, 120);
  phase2tkutil::add1DDesc(
      desc, "L1Stub_bendFE", "L1Stub_bendFE", "L1Stub_bendFE", "Trigger bend", "# L1 Stubs", 69, -8.625, 8.625);
  phase2tkutil::add1DDesc(
      desc, "L1Stub_bendBE", "L1Stub_bendBE", "L1Stub_bendBE", "Hardware bend", "# L1 Stubs", 69, -8.625, 8.625);
  phase2tkutil::add1DDesc(desc, "L1Stub_isPS", "L1Stub_isPS", "L1Stub_isPS", "Is PS?", "# L1 Stubs", 2, 0, 2);

  // Barrel Histos
  phase2tkutil::add1DDesc(
      desc, "Num_L1Stubs_Barrel", "Num_L1Stubs_Barrel", "Num_L1Stubs_Barrel", "Barrel Layer", "# L1 Stubs", 6, 0.5, 6.5);
  phase2tkutil::add2DDesc(desc,
                          "L1Stub_Width_Barrel",
                          "L1Stub_Width_Barrel",
                          "L1Stub_Width_Barrel",
                          "Barrel Layer",
                          "Displacement - Offset",
                          6,
                          0.5,
                          6.5,
                          43,
                          -10.75,
                          10.75);
  phase2tkutil::add2DDesc(desc,
                          "L1Stub_Offset_Barrel",
                          "L1Stub_Offset_Barrel",
                          "L1Stub_Offset_Barrel",
                          "Barrel Layer",
                          "Trigger Offset",
                          6,
                          0.5,
                          6.5,
                          43,
                          -10.75,
                          10.75);
  phase2tkutil::add1DDesc(desc,
                          "noise",
                          "Noise",
                          "Number of clusters - number of L1Stubs*2 in {} per event",
                          "Number of noise clusters",
                          "Number of events",
                          21,
                          -0.5,
                          20.5);

  // Layer histos
  phase2tkutil::add1DDesc(desc,
                          "NStubsLayer",
                          "Num_L1Stubs_Per_Event",
                          "Number of L1Stubs in {} per event",
                          "Number of stubs",
                          "Number of events",
                          61,
                          -0.5,
                          60.5);
  phase2tkutil::add2DDesc(desc,
                          "PositionOfClusters_2S",
                          "Position_Clusters_2S_module",
                          "Positions of clusters in 2S module",
                          "Strip",
                          "Half-module",
                          1016,
                          0.5,
                          1016.5,
                          5,
                          -2.5,
                          2.5);

  phase2tkutil::add1DDesc(desc,
                          "NClusters_Barrel",
                          "Num_Clusters_Barrel",
                          "Number of clusters per Barrel Layer",
                          "Barrel Layer",
                          "Number of clusters",
                          6,
                          0.5,
                          6.5);

  phase2tkutil::add2DDesc(desc,
                          "GlobalPositionXY_S",
                          "Cluster_Global_Position_XY_S",
                          "Cluster Position XY S",
                          "Cluster position x [cm]",
                          "Cluster position y [cm]",
                          14,
                          -7.0,
                          7.0,
                          60,
                          -10.0,
                          50.0);
  phase2tkutil::add2DDesc(desc,
                          "GlobalPositionRZ_S",
                          "Cluster_Global_Position_RZ_S",
                          "Cluster Position RZ S",
                          "Cluster position z [cm]",
                          "Cluster position #rho [cm]",
                          140,
                          -70.0,
                          70.0,
                          60,
                          0.0,
                          60.0);

  //Layer wise histos
  phase2tkutil::add1DDesc(desc,
                          "NClustersLayer_S",
                          "Num_Clusters_Per_Event_S",
                          "Number Of Clusters in strips in {}",
                          "Number of clusters per event (strip sensor)",
                          "",
                          61,
                          -0.5,
                          60.5);

  phase2tkutil::add1DDesc(desc,
                          "ClusterSize_P",
                          "Cluster_Size_P",
                          "Cluster Size in Pixels in {}",
                          "Cluster size (macro pixel sensor)",
                          "",
                          31,
                          -0.5,
                          30.5);
  phase2tkutil::add1DDesc(desc,
                          "ClusterSize_S",
                          "Cluster_Size_S",
                          "Cluster Size in strips in {}",
                          "Cluster size (strip sensor)",
                          "",
                          31,
                          -0.5,
                          30.5);

  phase2tkutil::add2DDesc(
      desc, "PositionOfClusters_2SLadder", "Position_Clusters_2S_Ladder", "", "", "", 13, -0.5, 12.5, 5, -2.5, 2.5);

    phase2tkutil::add2DDesc(desc,
                          "ClusterCrackOverview",
                          "Crack_Overview_OT_Cluster",
                          "Crack_Overview_OT_Clusters",
                          "Module",
                          "Layer",
                          0,
                          0.0,
                          13.0,
                          0,
                          0.0,
                          7.5);

  desc.add<std::string>("TopFolderName", "CRACK");
  desc.add<edm::InputTag>("clusterSrc", edm::InputTag("siPhase2Clusters"));
  desc.add<edm::InputTag>("TTStubs", edm::InputTag("TTStubsFromPhase2TrackerDigis", "StubAccepted"));
  descriptions.add("Phase2CRACKMonitor", desc);
}

DEFINE_FWK_MODULE(Phase2CRACKMonitor);
