#ifndef SiPixelTools_SiPixelGainCalibrationDBUploader_h
#define SiPixelTools_SiPixelGainCalibrationDBUploader_h

// -*- C++ -*-
//
// Package:    SiPixelGainCalibrationDBUploader
// Class:      SiPixelGainCalibrationDBUploader
// 
/**\class SiPixelGainCalibrationDBUploader SiPixelGainCalibrationDBUploader.cc CalibTracker/SiPixelGainCalibrationDBUploader/src/SiPixelGainCalibrationDBUploader.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Freya BLEKMAN
//         Created:  Tue Aug  5 16:22:46 CEST 2008
// $Id: SiPixelGainCalibrationDBUploader.h,v 1.2 2009/05/28 22:12:55 dlange Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
//#if CMSSW_VERSION >= 123
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
//#else
//#include "FWCore/Framework/interface/EDAnalyzer.h"
//#endif

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelCalibConfiguration.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelGainCalibration.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelGainCalibrationOffline.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelGainCalibrationForHLT.h"
#include "CalibTracker/SiPixelESProducers/interface/SiPixelGainCalibrationService.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"


#include "TH2F.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TString.h"
#include "TList.h"
//
// class decleration
//

//#if CMSSW_VERSION >= 123
class SiPixelGainCalibrationDBUploader : public edm::one::EDAnalyzer<edm::one::WatchRuns>
//#else
//class SiPixelGainCalibrationDBUploader : public edm::EDAnalyzer
//#endif
{
   public:
      explicit SiPixelGainCalibrationDBUploader(const edm::ParameterSet&);
      ~SiPixelGainCalibrationDBUploader();


   private:
      virtual void beginJob() ;
      virtual void beginRun(const edm::Run&, const edm::EventSetup&) override {};
      virtual void endRun(const edm::Run &, const edm::EventSetup &) override {};
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
  // functions added by F.B.
  void fillDatabase(const edm::EventSetup& iSetup);
  bool getHistograms();
      // ----------member data ---------------------------
  edm::ParameterSet conf_;
  std::map<uint32_t,std::map<std::string,TString> > bookkeeper_;
  std::map<uint32_t,std::map<double,double> > Meankeeper_;
  std::map<uint32_t,std::vector< std::map<int,int> > > noisyPixelsKeeper_;

  bool appendMode_;
  SiPixelGainCalibration *theGainCalibrationDbInput_;
  SiPixelGainCalibrationOffline *theGainCalibrationDbInputOffline_;
  //SiPixelGainCalibrationPhase1Offline *theGainCalibrationDbInputPhase1Offline_;
  SiPixelGainCalibrationForHLT *theGainCalibrationDbInputHLT_;
  SiPixelGainCalibrationService theGainCalibrationDbInputService_;
  TH2F *defaultGain_;
  TH2F *defaultPed_;
  TH2F *defaultChi2_;
  TH2F *defaultFitResult_;
  TH1F *meanGainHist_;
  TH1F *meanPedHist_;
  std::string record_;
  bool invertgain_;
  // keep track of lowest and highest vals for range
  float gainlow_;
  float gainhi_;
  float pedlow_;
  float pedhi_;
  bool usemeanwhenempty_;
  TFile *therootfile_;
  std::string rootfilestring_;
  float gainmax_;
  float pedmax_;
  double badchi2_;
  size_t nmaxcols;
  size_t nmaxrows;
  int countModulesFound;
  int badPed_, emptyPed_,badGain_,emptyGain_; 
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> trackerGeomToken_;
};

#endif
