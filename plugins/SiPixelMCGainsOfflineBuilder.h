#ifndef CondTools_SiPixel_SiPixelMCGainsOfflineBuilder_H
#define CondTools_SiPixel_SiPixelMCGainsOfflineBuilder_H
// -*- C++ -*-
//
// Package:    SiPixelMCGainsOfflineBuilder
// Class:      SiPixelMCGainsOfflineBuilder
//
/**\class SiPixelMCGainsOfflineBuilder SiPixelMCGainsOfflineBuilder.h SiPixel/test/SiPixelMCGainsOfflineBuilder.h

 Description: Test analyzer for writing pixel calibration in the DB

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Vincenzo CHIOCHIA
//         Created:  Tue Oct 17 17:40:56 CEST 2006
// $Id: SiPixelMCGainsOfflineBuilder.h,v 1.7 2009/11/20 19:21:29 rougny Exp $
//
//
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CalibTracker/SiPixelESProducers/interface/SiPixelGainCalibrationOfflineService.h"
#include "CondFormats/SiPixelObjects/interface/PixelIndices.h"
#include <string>

  class SiPixelMCGainsOfflineBuilder : public edm::EDAnalyzer {
  public:
    explicit SiPixelMCGainsOfflineBuilder(const edm::ParameterSet& iConfig);

    ~SiPixelMCGainsOfflineBuilder(){};
    virtual void beginJob();
    virtual void analyze(const edm::Event&, const edm::EventSetup&);
    virtual void endJob();
    bool loadFromFile();

  private:
    edm::ParameterSet conf_;
    bool appendMode_;
    SiPixelGainCalibrationOffline* SiPixelGainCalibration_;
    SiPixelGainCalibrationOfflineService SiPixelGainCalibrationService_;
    std::string recordName_;

    double meanPed_;
    double rmsPed_;
    double meanGain_;
    double rmsGain_;
    double meanPedFPix_;
    double rmsPedFPix_;
    double meanGainFPix_;
    double rmsGainFPix_;
    double deadFraction_;
    double noisyFraction_;
    double secondRocRowGainOffset_;
    double secondRocRowPedOffset_;
    int numberOfModules_;
    bool fromFile_;
    std::string fileName_;
    bool generateColumns_;
    double electronsPerVcal_;
    double electronsPerVcal_Offset_;
    double electronsPerVcal_L1_;
    double electronsPerVcal_L1_Offset_;

    // Internal class
    class CalParameters {
    public:
      float p0;
      float p1;
    };
    // Map for storing calibration constants
    std::map<int, CalParameters, std::less<int> > calmap_;
    PixelIndices* pIndexConverter_;  // Pointer to the index converter
  };

#endif
