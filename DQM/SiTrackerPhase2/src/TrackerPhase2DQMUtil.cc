#include "DQM/SiTrackerPhase2/interface/TrackerPhase2DQMUtil.h"

const std::vector<std::string> barrelName = {"Barrel/", "Barrel "};
const std::vector<std::string> endcapName = {"Endcaps/", "endcap "};
const std::vector<std::string> fPixName = {"ForwardPix/", "FPix "};
const std::vector<std::string> ePixName = {"EndcapPix/", "EPix "};
const std::vector<std::string> OTMinusName = {"MINUS/", "side minus "};
const std::vector<std::string> OTPlusName = {"PLUS/", "side plus "};
const std::vector<std::string> OTTEDD1Name = {"TEDD_1/", "TEDD 1 "};
const std::vector<std::string> OTTEDD2Name = {"TEDD_2/", "TEDD 2 "};
const int nFPixWheels = 8;
const int nEPixWheels = 4;
const int nFPixRings = 4;
const int nEPixRings = 5;
const int nTEDD1Wheels = 2;
const int nTEDD2Wheels = 3;
const int nTEDD1Rings = 15;
const int nTEDD2Rings = 12;

// Unified folder getter for IT and OT
// Gets the geographical information in either filepath or "pretty" format
// Uses the LEVEL to figure out which information to include
// LEVEL == 1: InnerTracker or OuterTracker (if LEVEL == 0, this is the behaviour)
// LEVEL == 2: Barrel or Endcap or Forward(IT)
// LEVEL == 3: Barrel or Endcap Shells (IT), Endcap Sides (OT)
// LEVEL == 4: Endcap rings
// LEVEL == 5: Endcap wheels
// LEVEL == 6: Barrel layers or endcap rings in wheels
std::string phase2tkutil::getHistoId(uint32_t det_id, const TrackerTopology* tTopo, float phi, int LEVEL, bool pretty) {
  std::ostringstream foldername;
  std::string Substructure, Side, Shell, TEDD;
  int layer = -1, wheel = -1, ring = -1;
  bool inner = (DetId(det_id).subdetId() == PixelSubdetector::PixelBarrel ||
                DetId(det_id).subdetId() == PixelSubdetector::PixelEndcap);

  if (DetId(det_id).subdetId() == PixelSubdetector::PixelBarrel ||
      DetId(det_id).subdetId() == SiStripSubdetector::TOB) {
    Substructure = barrelName[pretty];
    if (inner)
      layer = tTopo->getITPixelLayerNumber(det_id);
    else
      layer = tTopo->getOTLayerNumber(det_id);
  } else if (DetId(det_id).subdetId() == PixelSubdetector::PixelEndcap ||
             DetId(det_id).subdetId() == SiStripSubdetector::TID) {
    Substructure = endcapName[pretty];
    if (inner) {
      wheel = tTopo->pxfDisk(det_id);
      ring = tTopo->pxfBlade(det_id);

      if (wheel < 9)
        Substructure.append(fPixName[pretty]);
      else
        Substructure.append(ePixName[pretty]);

    } else {
      int side = tTopo->tidSide(det_id);
      Side = (side == 1 ? OTMinusName[pretty] : OTPlusName[pretty]);
      wheel = tTopo->tidWheel(det_id);
      TEDD = (wheel < 3 ? OTTEDD1Name[pretty] : OTTEDD2Name[pretty]);
      ring = tTopo->tidRing(det_id);
    }
  } else {  //unknown subdetector - should probably throw
    return "ERROR";
  }

  if (inner) {
    foldername << (pretty ? "IT " : "");
    Shell = getITShell(det_id, tTopo, phi);
  } else {
    foldername << (pretty ? "OT " : "");
  }

  if (LEVEL > 1)
    foldername << Substructure;

  if (LEVEL > 2) {
    if (inner)
      foldername << (pretty ? "shell " : "") << Shell << (pretty ? " " : "/");
    else if (DetId(det_id).subdetId() == SiStripSubdetector::TID)
      foldername << Side;
  }

  if (LEVEL == 4) {
    if (DetId(det_id).subdetId() == SiStripSubdetector::TID)
      foldername << TEDD << "Ring" << ring << (pretty ? " " : "/");
    else if (DetId(det_id).subdetId() == PixelSubdetector::PixelEndcap)
      foldername << "Ring" << ring << (pretty ? " " : "/");
  }

  if (LEVEL > 4) {
    if (DetId(det_id).subdetId() == PixelSubdetector::PixelEndcap)
      foldername << "Wheel" << wheel << (pretty ? " " : "/");
    else if (DetId(det_id).subdetId() == SiStripSubdetector::TID)
      foldername << TEDD << "Wheel" << wheel << (pretty ? " " : "/");
  }

  if (LEVEL > 5) {
    if (DetId(det_id).subdetId() == PixelSubdetector::PixelBarrel ||
        DetId(det_id).subdetId() == SiStripSubdetector::TOB)
      foldername << "Layer" << layer << (pretty ? " " : "/");
    else
      foldername << "Ring" << ring << (pretty ? " " : "/");
  }
  return foldername.str();
}

// Gets all possible folder names. Useful for validation/harvesting.
std::vector<std::string> phase2tkutil::getAllFolders() {
  std::vector<std::string> allFolders;
  std::string folderName;
  allFolders.push_back("");
  // IT
  const std::vector<std::string> shellNames = {"mO/", "mI/", "pO/", "pI/"};
  allFolders.push_back(barrelName[0]);
  allFolders.push_back(endcapName[0] + fPixName[0]);
  allFolders.push_back(endcapName[0] + ePixName[0]);
  for (int shell = 0; shell < 4; shell++) {
    allFolders.push_back(barrelName[0] + shellNames[shell]);
    for (int layer = 1; layer <= 4; layer++) {
      allFolders.push_back(barrelName[0] + shellNames[shell] + "Layer" + std::to_string(layer) + "/");
    }
    for (int endcapStructure = 1; endcapStructure <= 2; endcapStructure++) {
      for (int wheel = (endcapStructure == 1 ? 1 : nFPixWheels + 1);
           wheel <= (endcapStructure == 1 ? nFPixWheels : nFPixWheels + nEPixWheels);
           wheel++) {
        allFolders.push_back(endcapName[0] + (endcapStructure == 1 ? fPixName[0] : ePixName[0]) + shellNames[shell] +
                             "Wheel" + std::to_string(wheel) + "/");
        for (int ring = 1; ring <= (endcapStructure == 1 ? nFPixRings : nEPixRings); ring++) {
          allFolders.push_back(endcapName[0] + (endcapStructure == 1 ? fPixName[0] : ePixName[0]) + shellNames[shell] +
                               "Wheel" + std::to_string(wheel) + "/Ring" + std::to_string(ring) + "/");
        }
      }
      for (int ring = 1; ring <= (endcapStructure == 1 ? nFPixRings : nEPixRings); ring++) {
        allFolders.push_back(endcapName[0] + (endcapStructure == 1 ? fPixName[0] : ePixName[0]) + shellNames[shell] +
                             "Ring" + std::to_string(ring) + "/");
      }
    }
  }

  // OT
  for (int layer = 1; layer <= 6; layer++) {
    allFolders.push_back(barrelName[0] + "Layer" + std::to_string(layer) + "/");
  }
  allFolders.push_back(endcapName[0]);
  for (int side = 1; side <= 2; side++) {
    allFolders.push_back(endcapName[0] + (side == 1 ? OTMinusName[0] : OTPlusName[0]));
    for (int tedd = 1; tedd <= 2; tedd++) {
      for (int wheel = (tedd == 1 ? 1 : nTEDD1Wheels + 1);
           wheel <= (tedd == 1 ? nTEDD1Wheels : nTEDD1Wheels + nTEDD2Wheels);
           wheel++) {
        allFolders.push_back(endcapName[0] + (side == 1 ? OTMinusName[0] : OTPlusName[0]) +
                             (tedd == 1 ? OTTEDD1Name[0] : OTTEDD2Name[0]) + "Wheel" + std::to_string(wheel) + "/");
        for (int ring = 1; ring <= (tedd == 1 ? nTEDD1Rings : nTEDD2Rings); ring++) {
          allFolders.push_back(endcapName[0] + (side == 1 ? OTMinusName[0] : OTPlusName[0]) +
                               (tedd == 1 ? OTTEDD1Name[0] : OTTEDD2Name[0]) + "Wheel" + std::to_string(wheel) +
                               "/Ring" + std::to_string(ring) + "/");
        }
      }
      for (int ring = 1; ring <= (tedd == 1 ? nTEDD1Rings : nTEDD2Rings); ring++) {
        allFolders.push_back(endcapName[0] + (side == 1 ? OTMinusName[0] : OTPlusName[0]) +
                             (tedd == 1 ? OTTEDD1Name[0] : OTTEDD2Name[0]) + "Ring" + std::to_string(ring) + "/");
      }
    }
  }
  return allFolders;
}

std::string phase2tkutil::getITShell(uint32_t det_id, const TrackerTopology* tTopo, float phi) {
  std::string Side, Inner;
  std::ostringstream shellname;
  int layer = tTopo->getITPixelLayerNumber(det_id);
  if (DetId(det_id).subdetId() == PixelSubdetector::PixelBarrel) {
    if (layer % 2 == 0)
      Side = (tTopo->module(det_id) <= 5) ? "m" : "p";
    else
      Side = (tTopo->module(det_id) <= 4) ? "m" : "p";
  } else {
    int side = tTopo->tidSide(det_id);
    Side = (side == 1) ? "m" : "p";
  }
  Inner = (std::abs(phi) > 3.1415 / 2 ? "O" : "I");
  shellname << Side << Inner;
  return shellname.str();
}

int phase2tkutil::getITSignedModule(uint32_t det_id, const TrackerTopology* tTopo, float phi) {
  int signedModule;
  int module = tTopo->module(det_id);
  int layer = tTopo->getITPixelLayerNumber(det_id);
  if (layer % 2 == 0)
    signedModule = (module <= 5 ? module - 6 : module - 5);
  else
    signedModule = (module <= 4 ? module - 5 : module - 4);

  return signedModule;
}

int phase2tkutil::getITSignedLadder(uint32_t det_id, const TrackerTopology* tTopo, float phi) {
  int signedLadder;
  int ladder = tTopo->pxbLadder(det_id);
  int layer = tTopo->getITPixelLayerNumber(det_id);
  if (std::abs(phi) > 3.1415 / 2) {  // Outer shell
    if (layer == 1)
      signedLadder = ladder - 10;
    if (layer == 2)
      signedLadder = ladder - 19;
    if (layer == 3)
      signedLadder = ladder - 16;
    if (layer == 4)
      signedLadder = ladder - 22;
  } else {  // Inner shell
    if (layer == 1)
      signedLadder = (ladder > 9 ? ladder - 9 : ladder + 3);
    if (layer == 2)
      signedLadder = (ladder > 18 ? ladder - 18 : ladder + 6);
    if (layer == 3)
      signedLadder = (ladder > 15 ? ladder - 15 : ladder + 5);
    if (layer == 4)
      signedLadder = (ladder > 21 ? ladder - 21 : ladder + 7);
  }

  return signedLadder;
}

int phase2tkutil::getITSignedWheel(uint32_t det_id, const TrackerTopology* tTopo) {
  return (tTopo->tidSide(det_id) == 1 ? -tTopo->pxfDisk(det_id) : tTopo->pxfDisk(det_id));
}

typedef dqm::reco::MonitorElement MonitorElement;
typedef dqm::reco::DQMStore DQMStore;
MonitorElement* phase2tkutil::book1DFromPSet(const edm::ParameterSet& hpars,
                                             DQMStore::IBooker& ibooker,
                                             std::string titleString,
                                             int scale) {
  MonitorElement* temp = nullptr;
  if (hpars.getParameter<bool>("switch")) {
    double xMax = hpars.getParameter<double>("xmax");
    std::string title = hpars.getParameter<std::string>("title");
    xMax = xMax / scale;
    if (!titleString.empty())
      title = std::vformat(title, std::make_format_args(titleString));
    temp = ibooker.book1D(hpars.getParameter<std::string>("name"),
                          title,
                          hpars.getParameter<int32_t>("NxBins"),
                          hpars.getParameter<double>("xmin"),
                          xMax);
  }
  return temp;
}

MonitorElement* phase2tkutil::book2DFromPSet(const edm::ParameterSet& hpars,
                                             DQMStore::IBooker& ibooker,
                                             std::string titleString) {
  MonitorElement* temp = nullptr;
  if (hpars.getParameter<bool>("switch")) {
    std::string title = hpars.getParameter<std::string>("title");
    if (!titleString.empty())
      title = std::vformat(title, std::make_format_args(titleString));
    temp = ibooker.book2D(hpars.getParameter<std::string>("name"),
                          title,
                          hpars.getParameter<int32_t>("NxBins"),
                          hpars.getParameter<double>("xmin"),
                          hpars.getParameter<double>("xmax"),
                          hpars.getParameter<int32_t>("NyBins"),
                          hpars.getParameter<double>("ymin"),
                          hpars.getParameter<double>("ymax"));
  }
  return temp;
}

MonitorElement* phase2tkutil::bookProfile1DFromPSet(const edm::ParameterSet& hpars,
                                                    DQMStore::IBooker& ibooker,
                                                    std::string titleString) {
  MonitorElement* temp = nullptr;
  if (hpars.getParameter<bool>("switch")) {
    std::string title = hpars.getParameter<std::string>("title");
    if (!titleString.empty())
      title = std::vformat(title, std::make_format_args(titleString));
    temp = ibooker.bookProfile(hpars.getParameter<std::string>("name"),
                               title,
                               hpars.getParameter<int32_t>("NxBins"),
                               hpars.getParameter<double>("xmin"),
                               hpars.getParameter<double>("xmax"),
                               hpars.getParameter<double>("ymin"),
                               hpars.getParameter<double>("ymax"));
  }
  return temp;
}

void phase2tkutil::add1DDesc(edm::ParameterSetDescription& desc,
                             const std::string& psetKey,
                             const std::string& histName,
                             const std::string& histTitle,
                             const std::string& xlabel,
                             const std::string& ylabel,
                             int nbins,
                             double xmin,
                             double xmax) {
  edm::ParameterSetDescription ps;
  ps.add<bool>("switch", true);
  ps.add<std::string>("name", histName);
  ps.add<std::string>("title", histTitle + ";" + xlabel + ";" + ylabel);
  ps.add<int>("NxBins", nbins);
  ps.add<double>("xmin", xmin);
  ps.add<double>("xmax", xmax);
  desc.add<edm::ParameterSetDescription>(psetKey, ps);
}

void phase2tkutil::add2DDesc(edm::ParameterSetDescription& desc,
                             const std::string& psetKey,
                             const std::string& histName,
                             const std::string& histTitle,
                             const std::string& xlabel,
                             const std::string& ylabel,
                             int nbx,
                             double xmin,
                             double xmax,
                             int nby,
                             double ymin,
                             double ymax) {
  edm::ParameterSetDescription ps;
  ps.add<bool>("switch", true);
  ps.add<std::string>("name", histName);
  ps.add<std::string>("title", histTitle + ";" + xlabel + ";" + ylabel);
  ps.add<int>("NxBins", nbx);
  ps.add<double>("xmin", xmin);
  ps.add<double>("xmax", xmax);
  ps.add<int>("NyBins", nby);
  ps.add<double>("ymin", ymin);
  ps.add<double>("ymax", ymax);
  desc.add<edm::ParameterSetDescription>(psetKey, ps);
}
