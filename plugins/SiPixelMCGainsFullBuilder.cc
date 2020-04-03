// Uses the same mean & rms for both bpix & fpix
//
#include <memory>
#include <iostream>
#include "SiPixelMCGainsFullBuilder.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonTopologies/interface/PixelTopology.h"

#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CLHEP/Random/RandGauss.h"


  SiPixelMCGainsFullBuilder::SiPixelMCGainsFullBuilder(const edm::ParameterSet& iConfig)
    : conf_(iConfig),
      appendMode_(conf_.getUntrackedParameter<bool>("appendMode", true)),
      SiPixelGainCalibration_(0),
      SiPixelGainCalibrationService_(iConfig),
      recordName_(iConfig.getParameter<std::string>("record")),
      meanPed_(conf_.getParameter<double>("meanPed")),
      rmsPed_(conf_.getParameter<double>("rmsPed")),
      meanGain_(conf_.getParameter<double>("meanGain")),
      rmsGain_(conf_.getParameter<double>("rmsGain")),
      // Uses same mean and rms for bpix and fpix
      secondRocRowGainOffset_(conf_.getParameter<double>("secondRocRowGainOffset")),
    secondRocRowPedOffset_(conf_.getParameter<double>("secondRocRowPedOffset")),
    numberOfModules_(conf_.getParameter<int>("numberOfModules")),
    fromFile_(conf_.getParameter<bool>("fromFile")),
    fileName_(conf_.getParameter<std::string>("fileName")),
    electronsPerVcal_(conf_.getUntrackedParameter<double>("ElectronsPerVcal",1.)),
    electronsPerVcal_Offset_(conf_.getUntrackedParameter<double>("ElectronsPerVcal_Offset",0.)),
    electronsPerVcal_L1_(conf_.getUntrackedParameter<double>("ElectronsPerVcal_L1",1.)),
    electronsPerVcal_L1_Offset_(conf_.getUntrackedParameter<double>("ElectronsPerVcal_L1_Offset",0.))
    
  {
    ::putenv((char*)"CORAL_AUTH_USER=me");
    ::putenv((char*)"CORAL_AUTH_PASSWORD=test");
  }

  void SiPixelMCGainsFullBuilder::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;
    unsigned int run = iEvent.id().run();
    int nmodules = 0;
    uint32_t nchannels = 0;
    //    int mycol = 415;
    //    int myrow = 159;
    std::cout<<electronsPerVcal_<<" "<<electronsPerVcal_Offset_<<" "<<electronsPerVcal_L1_<<" "<<electronsPerVcal_L1_Offset_<<std::endl;
    edm::LogInfo("SiPixelMCGainsFullBuilder")
        << "... creating dummy SiPixelGainCalibration Data for Run " << run << "\n " << std::endl;
    //
    // Instantiate Gain calibration offset and define pedestal/gain range
    //
    float mingain = 0;
    float maxgain = 10;
    float minped = 0;
    float maxped = 255;
    if(electronsPerVcal_>1.) maxgain = maxgain * electronsPerVcal_;

    SiPixelGainCalibration_ = new SiPixelGainCalibration(minped, maxped, mingain, maxgain);

    edm::ESHandle<TrackerGeometry> pDD;
    iSetup.get<TrackerDigiGeometryRecord>().get(pDD);
    edm::LogInfo("SiPixelMCGainsFullBuilder") << " There are " << pDD->dets().size() << " detectors" << std::endl;
   //Retrieve tracker topology from geometry
    edm::ESHandle<TrackerTopology> tTopo;
    iSetup.get<TrackerTopologyRcd>().get(tTopo);
    //const TrackerTopology* tt = tTopo.product();

    for (TrackerGeometry::DetContainer::const_iterator it = pDD->dets().begin(); it != pDD->dets().end(); it++) {
      if (dynamic_cast<PixelGeomDetUnit const*>((*it)) != 0) {
        uint32_t detid = ((*it)->geographicalId()).rawId();

        // Stop if module limit reached
        nmodules++;
        if (nmodules > numberOfModules_)
          break;

        const PixelGeomDetUnit* pixDet = dynamic_cast<const PixelGeomDetUnit*>((*it));
        const PixelTopology& topol = pixDet->specificTopology();

	// Find out if it is layer 1
        //DetId detId = (*it)->geographicalId();
        DetId detId(detid);
        unsigned int layer = tTopo->pxbLayer(detId);

        // Get the module sizes.
        int nrows = topol.nrows();     // rows in x
        int ncols = topol.ncolumns();  // cols in y
        //std::cout << " ---> PIXEL DETID " << detid << " Cols " << ncols << " Rows " << nrows << std::endl;

        PixelIndices pIndexConverter(ncols, nrows);

        std::vector<char> theSiPixelGainCalibration;

        // Loop over columns and rows of this DetID
        for (int i = 0; i < ncols; i++) {
          for (int j = 0; j < nrows; j++) {
            nchannels++;

            float ped = 0.0, gain = 0.0;

            if (fromFile_) {
              // Use calibration from a file
              int chipIndex = 0, colROC = 0, rowROC = 0;

              pIndexConverter.transformToROC(i, j, chipIndex, colROC, rowROC);
              int chanROC = PixelIndices::pixelToChannelROC(rowROC, colROC);  // use ROC coordinates
              //	     float pp0=0, pp1=0;
              std::map<int, CalParameters, std::less<int> >::const_iterator it = calmap_.find(chanROC);
              CalParameters theCalParameters = (*it).second;
              ped = theCalParameters.p0;
              gain = theCalParameters.p1;

            } else {  // From python 

              if (rmsPed_ > 0) {
                ped = CLHEP::RandGauss::shoot(meanPed_, rmsPed_);
                while (minped > ped || maxped < ped)
                  ped = CLHEP::RandGauss::shoot(meanPed_, rmsPed_);

              } else
                ped = meanPed_;
              if (rmsGain_ > 0) {
                gain = CLHEP::RandGauss::shoot(meanGain_, rmsGain_);
                while (mingain > gain || maxgain < gain)
                  gain = CLHEP::RandGauss::shoot(meanGain_, rmsGain_);
              } else
                gain = meanGain_;
            }

            //if in the second row of rocs (i.e. a 2xN plaquette) add an offset (if desired) for testing
            //if (j >= 80) {
	    //ped += secondRocRowPedOffset_;
	    //gain += secondRocRowGainOffset_;

           // include the vcal claibration already here 

           double newGain = 1, newPed  = 0.;
	   if(layer==1) { 
	     newGain = gain * electronsPerVcal_L1_;
	     newPed  = ped  - (electronsPerVcal_L1_Offset_/newGain);
	   } else {
	     newGain = gain * electronsPerVcal_;
	     newPed  = ped  - (electronsPerVcal_Offset_/newGain);
	   }
	   ped = newPed;
	   gain = newGain;
	   
	   if (gain > maxgain) gain = maxgain;
	   else if (gain < mingain) gain = mingain;
	   if (ped > maxped) ped = maxped;
	   else if (ped < minped) ped = minped;
      
	   //std::cout<<gain<<" "<<ped<<" "<<newGain<<" "<<newPed<<std::endl;

	   // 	   if(i==mycol && j==myrow) {
	   //	     std::cout << "       Col "<<i<<" Row "<<j<<" Ped "<<ped<<" Gain "<<gain<<std::endl;
	   // 	   }	   
	   // 	   gain =  2.8;
	   // 	   ped  = 28.2;
	   
	   // Insert data in the container
	   SiPixelGainCalibration_->setData(ped, gain, theSiPixelGainCalibration);
          }
        }

        SiPixelGainCalibration::Range range(theSiPixelGainCalibration.begin(), theSiPixelGainCalibration.end());
        if (!SiPixelGainCalibration_->put(detid, range, ncols))
          edm::LogError("SiPixelMCGainsFullBuilder")
              << "[SiPixelMCGainsFullBuilder::analyze] detid already exists" << std::endl;
      }
    }
    std::cout << " ---> PIXEL Modules  " << nmodules << std::endl;
    std::cout << " ---> PIXEL Channels " << nchannels << std::endl;

    //   // Try to read object
    //    int mynmodules =0;
    //    for(TrackerGeometry::DetContainer::const_iterator it = pDD->dets().begin(); it != pDD->dets().end(); it++){
    //      if( dynamic_cast<PixelGeomDetUnit const*>((*it))!=0){
    //        uint32_t mydetid=((*it)->geographicalId()).rawId();
    //        mynmodules++;
    //        if( mynmodules > numberOfModules_) break;
    //        SiPixelGainCalibration::Range myrange = SiPixelGainCalibration_->getRange(mydetid);
    //        float mypedestal = SiPixelGainCalibration_->getPed (mycol,myrow,myrange,416);
    //        float mygain     = SiPixelGainCalibration_->getGain(mycol,myrow,myrange,416);
    //        //std::cout<<" PEDESTAL "<< mypedestal<<" GAIN "<<mygain<<std::endl;
    //      }
    //    }
    // Write into DB
    edm::LogInfo(" --- writeing to DB!");
    edm::Service<cond::service::PoolDBOutputService> mydbservice;
    if (!mydbservice.isAvailable()) {
      std::cout << "Didn't get DB" << std::endl;
      edm::LogError("db service unavailable");
      return;
    } else {
      edm::LogInfo("DB service OK");
    }

    try {
      //     size_t callbackToken = mydbservice->callbackToken("SiPixelGainCalibration");
      //     edm::LogInfo("SiPixelMCGainsFullBuilder")<<"CallbackToken SiPixelGainCalibration "
      //         <<callbackToken<<std::endl;
      //       unsigned long long tillTime;
      //     if ( appendMode_)
      //	 tillTime = mydbservice->currentTime();
      //       else
      //	 tillTime = mydbservice->endOfTime();
      //     edm::LogInfo("SiPixelMCGainsFullBuilder")<<"[SiPixelMCGainsFullBuilder::analyze] tillTime = "
      //         <<tillTime<<std::endl;
      //     mydbservice->newValidityForNewPayload<SiPixelGainCalibration>(
      //           SiPixelGainCalibration_, tillTime , callbackToken);

      if (mydbservice->isNewTagRequest(recordName_)) {
        mydbservice->createNewIOV<SiPixelGainCalibration>(
            SiPixelGainCalibration_, mydbservice->beginOfTime(), mydbservice->endOfTime(), recordName_);
      } else {
        mydbservice->appendSinceTime<SiPixelGainCalibration>(
            SiPixelGainCalibration_, mydbservice->currentTime(), recordName_);
      }
      edm::LogInfo(" --- all OK");
    } catch (const cond::Exception& er) {
      std::cout << "Database exception!   " << er.what() << std::endl;
      edm::LogError("SiPixelMCGainsFullBuilder") << er.what() << std::endl;
    } catch (const std::exception& er) {
      std::cout << "Standard exception!   " << er.what() << std::endl;
      edm::LogError("SiPixelMCGainsFullBuilder") << "caught std::exception " << er.what() << std::endl;
    } catch (...) {
      edm::LogError("SiPixelMCGainsFullBuilder") << "Funny error" << std::endl;
    }
  }

  // ------------ method called once each job just before starting event loop  ------------
  void SiPixelMCGainsFullBuilder::beginJob() {
    if (fromFile_) {
      if (loadFromFile()) {
        edm::LogInfo("SiPixelMCGainsFullBuilder") << " Calibration loaded: Map size " << calmap_.size() << " max "
                                              << calmap_.max_size() << " " << calmap_.empty() << std::endl;
      }
    }
  }

  // ------------ method called once each job just after ending the event loop  ------------
  void SiPixelMCGainsFullBuilder::endJob() {}

  bool SiPixelMCGainsFullBuilder::loadFromFile() {
    float par0, par1;  //,par2,par3;
    int colid, rowid;  //rocid
    std::string name;

    std::ifstream in_file;                          // data file pointer
    in_file.open(fileName_.c_str(), std::ios::in);  // in C++
    if (in_file.bad()) {
      edm::LogError("SiPixelMCGainsFullBuilder") << "Input file not found" << std::endl;
    }
    if (in_file.eof() != 0) {
      edm::LogError("SiPixelMCGainsFullBuilder") << in_file.eof() << " " << in_file.gcount() << " " << in_file.fail() << " "
                                             << in_file.good() << " end of file " << std::endl;
      return false;
    }
    //Load file header
    char line[500];
    for (int i = 0; i < 3; i++) {
      in_file.getline(line, 500, '\n');
      edm::LogInfo("SiPixelMCGainsFullBuilder") << line << std::endl;
    }
    //Loading calibration constants from file, loop on pixels
    for (int i = 0; i < (52 * 80); i++) {
      in_file >> par0 >> par1 >> name >> colid >> rowid;

      std::cout << " Col " << colid << " Row " << rowid << " P0 " << par0 << " P1 " << par1 << std::endl;

      CalParameters onePix;
      onePix.p0 = par0;
      onePix.p1 = par1;

      // Convert ROC pixel index to channel
      int chan = PixelIndices::pixelToChannelROC(rowid, colid);
      calmap_.insert(std::pair<int, CalParameters>(chan, onePix));
    }

    bool flag;
    if (calmap_.size() == 4160) {
      flag = true;
    } else {
      flag = false;
    }
    return flag;
  }

  //define this as a plug-in
  DEFINE_FWK_MODULE(SiPixelMCGainsFullBuilder);


