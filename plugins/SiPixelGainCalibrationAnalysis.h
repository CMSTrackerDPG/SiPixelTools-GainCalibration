// -*- C++ -*-
//
// Package:    SiPixelGainCalibrationAnalysis
// Class:      SiPixelGainCalibrationAnalysis
// 
/**\class SiPixelGainCalibrationAnalysis SiPixelGainCalibrationAnalysis.h CalibTracker/SiPixelGainCalibrationAnalysis/interface/SiPixelGainCalibrationAnalysis.h

Description: <one line class summary>

Implementation:
<Notes on implementation>
*/
//
// Original Author:  Freya Blekman
//         Created:  Wed Nov 14 15:02:06 CET 2007
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "DPGAnalysis-SiPixelTools/GainCalibration/plugins/SiPixelOfflineCalibAnalysisBase.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondFormats/SiPixelObjects/interface/SiPixelCalibConfiguration.h"
//#include "CondFormats/SiPixelObjects/interface/SiPixelGainCalibration.h"
//#include "CondFormats/SiPixelObjects/interface/SiPixelGainCalibrationOffline.h"
//#include "CondFormats/SiPixelObjects/interface/SiPixelGainCalibrationForHLT.h"

//#include "CalibTracker/SiPixelESProducers/interface/SiPixelGainCalibrationService.h"

#include "DQMServices/Core/interface/MonitorElement.h"
#include "TLinearFitter.h"
#include "TGraphErrors.h"
#include <fstream>



//
// class decleration
//

class SiPixelGainCalibrationAnalysis : public SiPixelOfflineCalibAnalysisBase {
public:
  explicit SiPixelGainCalibrationAnalysis(const edm::ParameterSet& iConfig);
  ~SiPixelGainCalibrationAnalysis();

  void doSetup(const edm::ParameterSet&);
  virtual bool doFits(uint32_t detid, std::vector<SiPixelCalibDigi>::const_iterator ipix);

  virtual bool checkCorrectCalibrationType();

private:
      
  virtual void calibrationSetup(const edm::EventSetup& iSetup);
      
  virtual void calibrationEnd();
  virtual void newDetID(uint32_t detid);
  void fillDatabase();
  void printSummary();
  std::vector<float> CalculateAveragePerColumn(uint32_t detid, std::string label);
  // ----------member data --------------------------- 
  edm::ParameterSet conf_;
  // more class members used to keep track of the histograms
  std::map<uint32_t,std::map<std::string, MonitorElement *> > bookkeeper_;
  std::map<uint32_t,std::map<std::string, MonitorElement *> > bookkeeper_pixels_;

  std::map<uint32_t,std::map<std::string,TH1F *> > bookkeeper_1D_;
  std::map<uint32_t,std::map<std::string,TH1F *> > bookkeeper_pixels_1D_;
  std::map<uint32_t,std::map<std::string,TH2F *> > bookkeeper_2D_;
  std::map<uint32_t,std::map<std::string,TH2F *> > bookkeeper_pixels_2D_;
  
  // std::map<std::string, TFileDirectory*>  myTFileDirMap;

  // fitter
  int nfitparameters_;
  std::string fitfunction_;
  TF1 *func_;
  TGraphErrors *graph_;
  
  //VCal to ele conversion map
  typedef  std::map<std::string, std::pair<double, double>> Maptype;
  Maptype VcalToEleMap;
  
  std::vector<uint32_t> listofdetids_;
  bool ignoreMode_;
  // flags

  bool reject_badpoints_;
  bool reject_plateaupoints_;
  bool reject_single_entries_;
  double plateau_max_slope_;
  bool reject_first_point_;
  double reject_badpoints_frac_;
  bool bookBIGCalibPayload_;
  bool savePixelHists_;
  double chi2Threshold_;
  double chi2ProbThreshold_;
  double maxGainInHist_;
  double maxChi2InHist_;
  bool saveALLHistograms_;
  bool sum_ped_cols_;
  bool sum_gain_cols_;
  bool filldb_;
  bool writeSummary_;
  
 
  
  // parameters for database output  
  std::string  recordName_;
  
  bool appendMode_;
  /*SiPixelGainCalibration *theGainCalibrationDbInput_;
  SiPixelGainCalibrationOffline *theGainCalibrationDbInputOffline_;
  SiPixelGainCalibrationForHLT *theGainCalibrationDbInputHLT_;
  SiPixelGainCalibrationService theGainCalibrationDbInputService_;*/

  // keep track of lowest and highest vals for range
  float gainlow_;
  float gainhi_;
  float pedlow_;
  float pedhi_;
  uint16_t min_nentries_;
  bool useVcalHigh_;
  double scalarVcalHigh_VcalLow_;
  
  // VCal to electrons conversion factor
  std::string  vCalToEleConvFactors_;
  
  //Summary
  std::ofstream summary_;
  uint32_t currentDetID_;
  int* statusNumbers_;
  
};
