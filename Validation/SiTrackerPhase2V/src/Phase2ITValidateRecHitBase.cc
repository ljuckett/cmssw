#include "FWCore/Framework/interface/ESWatcher.h"
#include "Validation/SiTrackerPhase2V/interface/Phase2ITValidateRecHitBase.h"
#include "Validation/SiTrackerPhase2V/interface/TrackerPhase2ValidationUtil.h"
#include "DQM/SiTrackerPhase2/interface/TrackerPhase2DQMUtil.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "Geometry/CommonTopologies/interface/GeomDet.h"
#include "Geometry/CommonTopologies/interface/TrackerGeomDet.h"
#include "Geometry/CommonTopologies/interface/PixelGeomDetUnit.h"
#include "Geometry/CommonTopologies/interface/PixelGeomDetType.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/GeometrySurface/interface/LocalError.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"

Phase2ITValidateRecHitBase::~Phase2ITValidateRecHitBase() = default;

Phase2ITValidateRecHitBase::Phase2ITValidateRecHitBase(const edm::ParameterSet& iConfig)
    : config_(iConfig),
      geomToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()),
      topoToken_(esConsumes<TrackerTopology, TrackerTopologyRcd, edm::Transition::BeginRun>()) {
  edm::LogInfo("Phase2ITValidateRecHitBase") << ">>> Construct Phase2ITValidateRecHitBase ";
}

void Phase2ITValidateRecHitBase::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  tkGeom_ = &iSetup.getData(geomToken_);
  tTopo_ = &iSetup.getData(topoToken_);
}
//
// -- Book Histograms
//
void Phase2ITValidateRecHitBase::bookHistograms(DQMStore::IBooker& ibooker,
                                                edm::Run const& iRun,
                                                edm::EventSetup const& iSetup) {
  std::string top_folder = config_.getParameter<std::string>("TopFolderName");
  edm::LogInfo("Phase2ITValidateRecHitBase") << " Booking Histograms in : " << top_folder;
  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;
  if (theTkDigiGeomWatcher.check(iSetup)) {
    for (auto const& det_u : tkGeom_->detUnits()) {
      //Always check TrackerNumberingBuilder before changing this part
      if (!(det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXB ||
            det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXEC))
        continue;
      unsigned int detId_raw = det_u->geographicalId().rawId();
      bookLayerHistos(ibooker, detId_raw, top_folder);
    }
  }
}

void Phase2ITValidateRecHitBase::bookLayerHistos(DQMStore::IBooker& ibooker, unsigned int det_id, std::string& subdir) {
  ibooker.cd();
  const GeomDet* geomDet = tkGeom_->idToDet(det_id);
  GlobalPoint detPos = geomDet->surface().toGlobal(Local2DPoint(0, 0));
  for (enum Level bookingDepth = IT; bookingDepth <= LAYER; bookingDepth = Level(bookingDepth + 1)) {
    // Validation: Only book at IT, SUBSTRUCTURE, and LAYER
    if (bookingDepth == SHELL || bookingDepth == ENDCAP_RING || bookingDepth == ENDCAP_WHEEL)
      continue;
    std::string key = phase2tkutil::getHistoId(det_id, tTopo_, detPos.phi(), bookingDepth, false);
    std::string prettyName = phase2tkutil::getHistoId(det_id, tTopo_, detPos.phi(), bookingDepth, true);
    if (layerMEs_.find(key) == layerMEs_.end()) {
      ibooker.cd();
      RecHitME local_histos;
      ibooker.setCurrentFolder(subdir);
      ibooker.setCurrentFolder(subdir + "/" + key);
      edm::LogInfo("Phase2ITValidateRecHit") << " Booking Histograms in : " << (subdir + "/" + key);

      local_histos.deltaX =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaX"), ibooker, prettyName);

      local_histos.deltaY =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaY"), ibooker, prettyName);

      local_histos.pullX =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("PullX"), ibooker, prettyName);

      local_histos.pullY =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("PullY"), ibooker, prettyName);

      local_histos.deltaPhi =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaPhi"), ibooker, prettyName);

      local_histos.deltaX_eta =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaX_eta"), ibooker, prettyName);

      local_histos.deltaX_phi =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaX_phi"), ibooker, prettyName);

      local_histos.deltaY_eta =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaY_eta"), ibooker, prettyName);

      local_histos.deltaY_phi =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("DeltaY_phi"), ibooker, prettyName);

      local_histos.deltaX_clsizex = phase2tkutil::bookProfile1DFromPSet(
          config_.getParameter<edm::ParameterSet>("DeltaX_clsizeX"), ibooker, prettyName);

      local_histos.deltaX_clsizey = phase2tkutil::bookProfile1DFromPSet(
          config_.getParameter<edm::ParameterSet>("DeltaX_clsizeY"), ibooker, prettyName);

      local_histos.deltaY_clsizex = phase2tkutil::bookProfile1DFromPSet(
          config_.getParameter<edm::ParameterSet>("DeltaY_clsizeX"), ibooker, prettyName);

      local_histos.deltaY_clsizey = phase2tkutil::bookProfile1DFromPSet(
          config_.getParameter<edm::ParameterSet>("DeltaY_clsizeY"), ibooker, prettyName);

      local_histos.deltaYvsdeltaX = phase2tkutil::book2DFromPSet(
          config_.getParameter<edm::ParameterSet>("DeltaY_vs_DeltaX"), ibooker, prettyName);

      local_histos.pullX_eta = phase2tkutil::bookProfile1DFromPSet(
          config_.getParameter<edm::ParameterSet>("PullX_eta"), ibooker, prettyName);

      local_histos.pullY_eta = phase2tkutil::bookProfile1DFromPSet(
          config_.getParameter<edm::ParameterSet>("PullY_eta"), ibooker, prettyName);
      ibooker.setCurrentFolder(subdir + "/" + key + "/PrimarySimHits");
      //all histos for Primary particles
      local_histos.numberRecHitsprimary = phase2tkutil::book1DFromPSet(
          config_.getParameter<edm::ParameterSet>("nRecHits_primary"), ibooker, prettyName);

      edm::ParameterSet histoPSet = config_.getParameter<edm::ParameterSet>("DeltaX");
      histoPSet.addParameter<std::string>("title", "Delta X of primary SimHits in {};#delta x [#mum];");
      local_histos.deltaX_primary = phase2tkutil::book1DFromPSet(histoPSet, ibooker, prettyName);

      histoPSet = config_.getParameter<edm::ParameterSet>("DeltaY");
      histoPSet.addParameter<std::string>("title", "Delta Y of primary SimHits in {};#delta y [#mum];");
      local_histos.deltaY_primary = phase2tkutil::book1DFromPSet(histoPSet, ibooker, prettyName);

      histoPSet = config_.getParameter<edm::ParameterSet>("PullX");
      histoPSet.addParameter<std::string>("title", "Pull X of primary SimHits in {};pull x;");
      local_histos.pullX_primary = phase2tkutil::book1DFromPSet(histoPSet, ibooker, prettyName);

      histoPSet = config_.getParameter<edm::ParameterSet>("PullY");
      histoPSet.addParameter<std::string>("title", "Pull Y of primary SimHits in {};pull y;");
      local_histos.pullY_primary = phase2tkutil::book1DFromPSet(histoPSet, ibooker, prettyName);

      layerMEs_.emplace(key, local_histos);
    }
  }
}

void Phase2ITValidateRecHitBase::fillRechitHistos(const PSimHit* simhitClosest,
                                                  const SiPixelRecHit* rechit,
                                                  const std::map<unsigned int, SimTrack>& selectedSimTrackMap) {
  auto id = rechit->geographicalId();
  const GeomDet* geomDet = tkGeom_->idToDet(id);
  GlobalPoint detPos = geomDet->surface().toGlobal(Local2DPoint(0, 0));
  const GeomDetUnit* geomDetunit(tkGeom_->idToDetUnit(id));
  if (!geomDetunit)
    return;

  LocalPoint lp = rechit->localPosition();
  auto simTrackIt(selectedSimTrackMap.find(simhitClosest->trackId()));
  bool isPrimary = false;
  //check if simhit is primary
  if (simTrackIt != selectedSimTrackMap.end())
    isPrimary = phase2tkutil::isPrimary(simTrackIt->second, simhitClosest);

  Local3DPoint simlp(simhitClosest->localPosition());
  const LocalError& lperr = rechit->localPositionError();
  double dx = phase2tkutil::cmtomicron * (lp.x() - simlp.x());
  double dy = phase2tkutil::cmtomicron * (lp.y() - simlp.y());
  double pullx = 999.;
  double pully = 999.;
  if (lperr.xx())
    pullx = (lp.x() - simlp.x()) / std::sqrt(lperr.xx());
  if (lperr.yy())
    pully = (lp.y() - simlp.y()) / std::sqrt(lperr.yy());
  float eta = geomDetunit->surface().toGlobal(lp).eta();
  float phi = geomDetunit->surface().toGlobal(lp).phi();
  float dphi = phi - geomDetunit->surface().toGlobal(simlp).phi();

  for (enum Level fillingDepth = IT; fillingDepth <= LAYER; fillingDepth = Level(fillingDepth + 1)) {
    // Skip filling for barrel detIds on endcap-only depths
    if ((fillingDepth == ENDCAP_RING || fillingDepth == ENDCAP_WHEEL) &&
        DetId(id.rawId()).subdetId() == PixelSubdetector::PixelBarrel)
      continue;
    std::string key = phase2tkutil::getHistoId(id.rawId(), tTopo_, detPos.phi(), fillingDepth, false);
    if (layerMEs_[key].deltaX)
      layerMEs_[key].deltaX->Fill(dx);
    if (layerMEs_[key].deltaY)
      layerMEs_[key].deltaY->Fill(dy);
    if (layerMEs_[key].pullX)
      layerMEs_[key].pullX->Fill(pullx);
    if (layerMEs_[key].pullY)
      layerMEs_[key].pullY->Fill(pully);
    if (layerMEs_[key].deltaPhi)
      layerMEs_[key].deltaPhi->Fill(dphi);

    if (layerMEs_[key].deltaX_eta)
      layerMEs_[key].deltaX_eta->Fill(std::abs(eta), dx);
    if (layerMEs_[key].deltaY_eta)
      layerMEs_[key].deltaY_eta->Fill(std::abs(eta), dy);
    if (layerMEs_[key].deltaX_phi)
      layerMEs_[key].deltaX_phi->Fill(phi, dx);
    if (layerMEs_[key].deltaY_phi)
      layerMEs_[key].deltaY_phi->Fill(phi, dy);

    if (layerMEs_[key].deltaX_clsizex)
      layerMEs_[key].deltaX_clsizex->Fill(rechit->cluster()->sizeX(), dx);
    if (layerMEs_[key].deltaX_clsizey)
      layerMEs_[key].deltaX_clsizey->Fill(rechit->cluster()->sizeY(), dx);
    if (layerMEs_[key].deltaY_clsizex)
      layerMEs_[key].deltaY_clsizex->Fill(rechit->cluster()->sizeX(), dy);
    if (layerMEs_[key].deltaY_clsizey)
      layerMEs_[key].deltaY_clsizey->Fill(rechit->cluster()->sizeY(), dy);
    if (layerMEs_[key].deltaYvsdeltaX)
      layerMEs_[key].deltaYvsdeltaX->Fill(dx, dy);
    if (layerMEs_[key].pullX_eta)
      layerMEs_[key].pullX_eta->Fill(eta, pullx);
    if (layerMEs_[key].pullY_eta)
      layerMEs_[key].pullY_eta->Fill(eta, pully);
    if (isPrimary) {
      if (layerMEs_[key].deltaX_primary)
        layerMEs_[key].deltaX_primary->Fill(dx);
      if (layerMEs_[key].deltaY_primary)
        layerMEs_[key].deltaY_primary->Fill(dy);
      if (layerMEs_[key].pullX_primary)
        layerMEs_[key].pullX_primary->Fill(pullx);
      if (layerMEs_[key].pullY_primary)
        layerMEs_[key].pullY_primary->Fill(pully);
      layerMEs_[key].primaryRecHitCounter++;
    }
  }
}

void Phase2ITValidateRecHitBase::fillPSetDescription(edm::ParameterSetDescription& desc, bool tracking) {
  // TrackingRecHits have a larger range of delta phi values
  // The ranges are changed so validators can see the difference
  double delta_phi_range = tracking ? 0.5 : 0.005;
  std::string product_name = tracking ? "TrackingRecHits" : "RecHits";

  phase2tkutil::add1DDesc(desc,
                          "DeltaX",
                          "Delta_X",
                          "Delta X of " + product_name + " in {}",
                          "RecHit resolution X coordinate [#mum]",
                          "",
                          100,
                          -100.0,
                          100.0);
  phase2tkutil::add1DDesc(desc,
                          "DeltaY",
                          "Delta_Y",
                          "Delta Y of " + product_name + " in {}",
                          "RecHit resolution Y coordinate [#mum]",
                          "",
                          100,
                          -100.0,
                          100.0);
  phase2tkutil::add1DDesc(
      desc, "PullX", "Pull_X", "Pull X of " + product_name + " in {}", "Pull x", "", 100, -4.0, 4.0);
  phase2tkutil::add1DDesc(
      desc, "PullY", "Pull_Y", "Pull Y of " + product_name + " in {}", "Pull y", "", 100, -4.0, 4.0);
  phase2tkutil::add1DDesc(desc,
                          "DeltaPhi",
                          "Delta_Phi",
                          "Delta phi of " + product_name + " in {}",
                          "phi",
                          "",
                          100,
                          -delta_phi_range,
                          delta_phi_range);

  edm::ParameterSetDescription psd4;
  psd4.add<std::string>("name", "Delta_X_vs_Eta");
  psd4.add<std::string>("title", "Delta_X_vs_Eta;|#eta|;#Delta x [#mum]");
  psd4.add<int>("NyBins", 100);
  psd4.add<double>("ymin", -100.0);
  psd4.add<double>("ymax", 100.0);
  psd4.add<int>("NxBins", 41);
  psd4.add<bool>("switch", true);
  psd4.add<double>("xmax", 4.1);
  psd4.add<double>("xmin", 0.);
  desc.add<edm::ParameterSetDescription>("DeltaX_eta", psd4);

  edm::ParameterSetDescription psd4_y;
  psd4_y.add<std::string>("name", "Delta_X_vs_Phi");
  psd4_y.add<std::string>("title", "Delta_X_vs_Phi;#phi;#Delta x [#mum]");
  psd4_y.add<int>("NyBins", 100);
  psd4_y.add<double>("ymin", -100.0);
  psd4_y.add<double>("ymax", 100.0);
  psd4_y.add<int>("NxBins", 36);
  psd4_y.add<bool>("switch", true);
  psd4_y.add<double>("xmax", M_PI);
  psd4_y.add<double>("xmin", -M_PI);
  desc.add<edm::ParameterSetDescription>("DeltaX_phi", psd4_y);

  edm::ParameterSetDescription psd5;
  psd5.add<std::string>("name", "Delta_Y_vs_Eta");
  psd5.add<std::string>("title", "Delta_Y_vs_Eta;|#eta|;#Delta y [#mum]");
  psd5.add<int>("NyBins", 100);
  psd5.add<double>("ymin", -100.0);
  psd5.add<double>("ymax", 100.0);
  psd5.add<int>("NxBins", 41);
  psd5.add<bool>("switch", true);
  psd5.add<double>("xmax", 4.1);
  psd5.add<double>("xmin", 0.);
  desc.add<edm::ParameterSetDescription>("DeltaY_eta", psd5);

  edm::ParameterSetDescription psd5_y;
  psd5_y.add<std::string>("name", "Delta_Y_vs_Phi");
  psd5_y.add<std::string>("title", "Delta_Y_vs_Phi;#phi;#Delta y [#mum]");
  psd5_y.add<int>("NyBins", 100);
  psd5_y.add<double>("ymin", -100.0);
  psd5_y.add<double>("ymax", 100.0);
  psd5_y.add<int>("NxBins", 36);
  psd5_y.add<bool>("switch", true);
  psd5_y.add<double>("xmax", M_PI);
  psd5_y.add<double>("xmin", -M_PI);
  desc.add<edm::ParameterSetDescription>("DeltaY_phi", psd5_y);

  phase2tkutil::add2DDesc(desc,
                          "DeltaX_clsizeX",
                          "Delta_X_vs_ClusterSizeX",
                          "Delta X of " + product_name + " vs cluster size X in {}",
                          "Cluster size x",
                          "Delta x [#mum]",
                          21,
                          -0.5,
                          20.5,
                          200,
                          -100.0,
                          100.0);
  phase2tkutil::add2DDesc(desc,
                          "DeltaX_clsizeY",
                          "Delta_X_vs_ClusterSizeY",
                          "Delta X of " + product_name + " vs cluster size Y in {}",
                          "Cluster size y",
                          "Delta x [#mum]",
                          21,
                          -0.5,
                          20.5,
                          200,
                          -100.0,
                          100.0);
  phase2tkutil::add2DDesc(desc,
                          "DeltaY_clsizeX",
                          "Delta_Y_vs_ClusterSizeX",
                          "Delta Y of " + product_name + " vs cluster size X in {}",
                          "Cluster size x",
                          "Delta y [#mum]",
                          21,
                          -0.5,
                          20.5,
                          200,
                          -100.0,
                          100.0);
  phase2tkutil::add2DDesc(desc,
                          "DeltaY_clsizeY",
                          "Delta_Y_vs_ClusterSizeY",
                          "Delta Y of " + product_name + " vs cluster size Y in {}",
                          "Cluster size y",
                          "Delta y [#mum]",
                          21,
                          -0.5,
                          20.5,
                          200,
                          -100.0,
                          100.0);

  phase2tkutil::add2DDesc(desc,
                          "DeltaY_vs_DeltaX",
                          "Delta_Y_vs_Delta_X",
                          "Delta Y vs Delta X of " + product_name + " in {}",
                          "#Delta x [#mum]",
                          "#Delta y [#mum]",
                          100,
                          -100.0,
                          100.0,
                          100,
                          -100.0,
                          100.0);
  phase2tkutil::add2DDesc(desc,
                          "PullX_eta",
                          "Pull_X_vs_Eta",
                          "Pull x vs eta of " + product_name + " in {}",
                          "#eta",
                          "pull x",
                          82,
                          -4.1,
                          4.1,
                          82,
                          -4.1,
                          4.1);
  phase2tkutil::add2DDesc(desc,
                          "PullY_eta",
                          "Pull_Y_vs_Eta",
                          "Pull y vs eta of " + product_name + " in {}",
                          "#eta",
                          "pull y",
                          82,
                          -4.1,
                          4.1,
                          82,
                          -4.1,
                          4.1);

  phase2tkutil::add1DDesc(desc,
                          "nRecHits_primary",
                          "Num_RecHits_matched_primary_SimTrack",
                          "Number of " + product_name + " matched to primary SimTrack in {}",
                          "Number of rechits",
                          "Number of events",
                          100,
                          0.0,
                          1000.0);
}
