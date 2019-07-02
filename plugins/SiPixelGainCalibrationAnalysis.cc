// -*- C++ -*-
//
// Package:    SiPixelGainCalibrationAnalysis
// Class:      SiPixelGainCalibrationAnalysis
// 
/**\class SiPixelGainCalibrationAnalysis SiPixelGainCalibrationAnalysis.cc CalibTracker/SiPixelGainCalibrationAnalysis/src/SiPixelGainCalibrationAnalysis.cc

Description: <one line class summary>

Implementation:
<Notes on implementation>
*/
//
// Original Author:  Freya Blekman
//         Created:  Wed Nov 14 15:02:06 CET 2007
//
//

// user include files
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

#include "SiPixelGainCalibrationAnalysis.h"
#include <sstream>
#include <vector>
#include <math.h>
#include "TGraphErrors.h"
#include "TMath.h"

#include <iostream>
#include <fstream>
#include <string>

using std::cout;
using std::endl;

//
// constructors and destructor
//
SiPixelGainCalibrationAnalysis::SiPixelGainCalibrationAnalysis(const edm::ParameterSet& iConfig):
  SiPixelOfflineCalibAnalysisBase(iConfig),
  conf_(iConfig),
  bookkeeper_(),
  bookkeeper_pixels_(),
  bookkeeper_1D_(),
  bookkeeper_pixels_1D_(),
  bookkeeper_2D_(),
  bookkeeper_pixels_2D_(),
 

  nfitparameters_(iConfig.getUntrackedParameter<int>("numberOfFitParameters",2)),
  fitfunction_(iConfig.getUntrackedParameter<std::string>("fitFunctionRootFormula","pol1")),
  listofdetids_(conf_.getUntrackedParameter<std::vector<uint32_t> >("listOfDetIDs")),
  ignoreMode_(conf_.getUntrackedParameter<bool>("ignoreMode",false)),
  reject_plateaupoints_(iConfig.getUntrackedParameter<bool>("suppressPlateauInFit",true)),
  reject_single_entries_(iConfig.getUntrackedParameter<bool>("suppressPointsWithOneEntryOrLess",true)),
  plateau_max_slope_(iConfig.getUntrackedParameter<double>("plateauSlopeMax",1.0)),
  reject_first_point_(iConfig.getUntrackedParameter<bool>("rejectVCalZero",true)),
  reject_badpoints_frac_(iConfig.getUntrackedParameter<double>("suppressZeroAndPlateausInFitFrac",0)),
  bookBIGCalibPayload_(iConfig.getUntrackedParameter<bool>("saveFullPayloads",false)),
  savePixelHists_(iConfig.getUntrackedParameter<bool>("savePixelLevelHists",false)),
  chi2Threshold_(iConfig.getUntrackedParameter<double>("minChi2NDFforHistSave",10)),
  chi2ProbThreshold_(iConfig.getUntrackedParameter<double>("minChi2ProbforHistSave",0.05)),
  maxGainInHist_(iConfig.getUntrackedParameter<double>("maxGainInHist",10)),
  maxChi2InHist_(iConfig.getUntrackedParameter<double>("maxChi2InHist",25)),
  saveALLHistograms_(iConfig.getUntrackedParameter<bool>("saveAllHistograms",false)),
 

  filldb_(iConfig.getUntrackedParameter<bool>("writeDatabase",false)),
  writeSummary_(iConfig.getUntrackedParameter<bool>("writeSummary",true)),
  recordName_(conf_.getParameter<std::string>("record")),

  appendMode_(conf_.getUntrackedParameter<bool>("appendMode",true)),
  /*theGainCalibrationDbInput_(0),
  theGainCalibrationDbInputOffline_(0),
  theGainCalibrationDbInputHLT_(0),
  theGainCalibrationDbInputService_(iConfig),*/
  gainlow_(10.),gainhi_(0.),pedlow_(255.),pedhi_(0.),
  useVcalHigh_(conf_.getParameter<bool>("useVCALHIGH")),
  scalarVcalHigh_VcalLow_(conf_.getParameter<double>("vcalHighToLowConversionFac")),
  vCalToEleConvFactors_(conf_.getParameter<std::string>("vCalToEleConvFactors"))
{
  if(reject_single_entries_)
    min_nentries_=1;
  else
    min_nentries_=0;
  ::putenv((char*)"CORAL_AUTH_USER=me");
  ::putenv((char*)"CORAL_AUTH_PASSWORD=test");   
  edm::LogInfo("SiPixelGainCalibrationAnalysis") << "now using fit function " << fitfunction_ << ", which has " << nfitparameters_ << " free parameters. " << std::endl;
 
  func_= new TF1("func",fitfunction_.c_str(),0,256*scalarVcalHigh_VcalLow_);
  graph_ = new TGraphErrors();
  currentDetID_=0;
  summary_.open("SummaryPerDetID.txt");
  statusNumbers_ = new int[10];
  for(int ii=0;ii<10;ii++)
    statusNumbers_[ii]=0;
  
  
  //Define VCal to ele conversion factors
  std::ifstream fin;
  fin.open(vCalToEleConvFactors_);
  if(fin.is_open()){
     std::cout << "File ./" << vCalToEleConvFactors_ << " is open.\n";
     for(std::string line; std::getline(fin, line); ) {
       std::istringstream in(line);      //make a stream for the line itself
       std::string type;
       double VCslope_, VCoffset_;
       in >> type;
       
       in >> VCslope_ >> VCoffset_; 
       VcalToEleMap[type] = std::make_pair(VCslope_,VCoffset_);
     }
     fin.close();
   }
   else
    std::cout << "Error opening " << vCalToEleConvFactors_ << ". Are you sure you passed it correctly?\n";
  
   std::map<std::string, std::pair<double, double>>::iterator it = VcalToEleMap.begin();
   std::cout << "Using the following VCal to electron conversion factors per layer/ring (Rp = ring plus, Rm = ring minus, L = Layer):\n";
   while(it != VcalToEleMap.end()){
     std::pair<double, double> val = it->second;
     std::cout<<it->first<<" : Slope =  "<<val.first<<"  Offset = "<<val.second<<" "<<std::endl;
     it++;
   } 
      
}

SiPixelGainCalibrationAnalysis::~SiPixelGainCalibrationAnalysis()
{
}
// member functions
//
// ------------ method called once each job just before starting event loop  ------------

std::vector<float> SiPixelGainCalibrationAnalysis::CalculateAveragePerColumn(uint32_t detid, std::string label){
  std::vector<float> result;
  int ncols= bookkeeper_2D_[detid][label]->GetNbinsX();
  int nrows= bookkeeper_2D_[detid][label]->GetNbinsY();
  for(int icol=1; icol<=ncols; ++icol){
    float val=0;
    float ntimes =0;
    for(int irow=1; irow<=nrows; ++irow){
      val+= bookkeeper_2D_[detid][label]->GetBinContent(icol,irow);
      ntimes++;
    }
    val/= ntimes;
    result.push_back(val);
  }
  return result;


}

bool
SiPixelGainCalibrationAnalysis::checkCorrectCalibrationType()
{
  if(calibrationMode_=="GainCalibration")
    return true;
  else if(ignoreMode_==true)
    return true;
  else if(calibrationMode_=="unknown"){
    edm::LogInfo("SiPixelGainCalibrationAnalysis") <<  "calibration mode is: " << calibrationMode_ << ", continuing anyway..." ;

    return true;
  }
  else{
    //    edm::LogError("SiPixelGainCalibrationAnalysis") << "unknown calibration mode for Gain calibration, should be \"Gain\" and is \"" << calibrationMode_ << "\"";
  }
  return false;
}

void SiPixelGainCalibrationAnalysis::calibrationSetup(const edm::EventSetup&)
{
}
//------- summary printing method. Very verbose.
void
SiPixelGainCalibrationAnalysis::printSummary(){

  uint32_t detid=0;
  for(std::map<uint32_t,std::map<std::string,TH2F *> >::const_iterator idet = bookkeeper_2D_.begin(); idet != bookkeeper_2D_.end(); ++idet){
    if(detid==idet->first)
      continue;// only do things once per detid
    detid=idet->first;
    std::vector<float> gainvec=CalculateAveragePerColumn(detid,"gain_2d");
    std::vector<float> pedvec =CalculateAveragePerColumn(detid,"ped_2d");
    std::vector<float> chi2vec = CalculateAveragePerColumn(detid,"chi2_2d");
    std::ostringstream summarytext;
    
    summarytext << "Summary for det ID " << detid << "(" << translateDetIdToString(detid) << ")\n";
    summarytext << "\t Following: values per column: column #, gain, pedestal, chi2\n";
    for(uint32_t i=0; i<gainvec.size(); i++)
      summarytext << "\t " << i << " \t" << gainvec[i] << " \t" << pedvec[i] << " \t" << chi2vec[i] << "\n";
    summarytext << "\t list of pixels with high chi2 (chi2> " << chi2Threshold_ << "): \n";
    
    
    for(std::map<std::string, TH1F *>::const_iterator ipix = bookkeeper_pixels_1D_[detid].begin(); ipix!=bookkeeper_pixels_1D_[detid].end(); ++ipix)
      summarytext << "\t " << ipix->first << "\n";
    edm::LogInfo("SiPixelGainCalibrationAnalysis") << summarytext.str() << std::endl;
    
  }
  if(summary_.is_open()){
    summary_.close();
    summary_.open("Summary.txt");
    summary_<<"Total Number of Pixel computed :"<<statusNumbers_[9]<<endl;
    summary_<<"Number of pixel tagged with status :"<<endl;
    for(int ii=0;ii<9;ii++)
      summary_<<ii<<" -> "<<statusNumbers_[ii]<<" ~ "<<double(statusNumbers_[ii])/double(statusNumbers_[9])*100.<<" %"<<endl;
    
    summary_.close();
  
  }

}

// ------------ method called once each job just after ending the event loop  ------------

void 
SiPixelGainCalibrationAnalysis::calibrationEnd() {

  if(writeSummary_) printSummary();
  
  // this is where we loop over all histograms and save the database objects
  if(filldb_)
    fillDatabase();
}
//-----------method to fill the database
void SiPixelGainCalibrationAnalysis::fillDatabase(){
 // only create when necessary.
  // process the minimum and maximum gain & ped values...
  edm::LogError("SiPixelGainCalibration::fillDatabase()") << "PLEASE do not fill the database directly from the gain calibration analyzer. This function is currently disabled and no DB payloads will be produced!" << std::endl;

}
// ------------ method called to do fits to all objects available  ------------
bool
SiPixelGainCalibrationAnalysis::doFits(uint32_t detid, std::vector<SiPixelCalibDigi>::const_iterator ipix,std::string layerString)
{
 
  double VcalToEle_slope  ;
  double VcalToEle_offset ;
  
  if (!layerString.empty()){
    VcalToEle_slope  = VcalToEleMap[layerString].first;
    VcalToEle_offset = VcalToEleMap[layerString].second;
  }
  else{
    VcalToEle_slope  = 1.;
    VcalToEle_offset = 0.;
  }
  
  std::string currentDir = GetPixelDirectory(detid);
  float lowmeanval=255;
  float highmeanval=0;
  bool makehistopersistent = saveALLHistograms_;
  std::vector<uint32_t>::const_iterator detidfinder=find(listofdetids_.begin(),listofdetids_.end(),detid);
  if(detidfinder!=listofdetids_.end())
    makehistopersistent=true;
  // first, fill the input arrays to the TLinearFitter.
  double xvals[257];
  double yvals[256];
  double yerrvals[256];
  double xvalsall[257];
  double yvalsall[256];
  double yerrvalsall[256];
  int npoints=0;
  int nallpoints=0;
  bool use_point=true;
  int status=0;
  statusNumbers_[9]++;
  bookkeeper_2D_[detid]["status_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,0);
  if(writeSummary_ && detid!=currentDetID_){
    currentDetID_ = detid;
    summary_<<endl<<"DetId_"<<currentDetID_<<endl;
  }



  
  
  for(uint32_t ii=0; ii< ipix->getnpoints() && ii<200; ii++){
    //    std::cout << ipix->getsum(ii) << " " << ipix->getnentries(ii) << " " << ipix->getsumsquares(ii) << std::endl;
    nallpoints++;
    use_point=true;
    if(useVcalHigh_){
      xvalsall[ii]=vCalValues_[ii]*scalarVcalHigh_VcalLow_;
    }
    else
      xvalsall[ii]=vCalValues_[ii];
    
    xvalsall[ii]= xvalsall[ii]*VcalToEle_slope+VcalToEle_offset;
    yerrvalsall[ii]=yvalsall[ii]=0; 
  
    if(ipix->getnentries(ii)>min_nentries_){
      yvalsall[ii]=ipix->getsum(ii)/(float)ipix->getnentries(ii);
      yerrvalsall[ii]=ipix->getsumsquares(ii)/(float)(ipix->getnentries(ii));
      yerrvalsall[ii]-=pow(yvalsall[ii],2);
      yerrvalsall[ii]=sqrt(yerrvalsall[ii])/sqrt(ipix->getnentries(ii));
      
      if(yvalsall[ii]<lowmeanval)
	lowmeanval=yvalsall[ii];
      if(yvalsall[ii]>highmeanval)
	highmeanval=yvalsall[ii];
    }
  }
  
  // calculate plateau value from last 4 entries
  double plateauval=0;
  bool noPlateau=0;
  if(nallpoints>=4){
    for(int ii=nallpoints-1; ii>nallpoints-5; --ii) plateauval+=yvalsall[ii];
    plateauval/=4;
    for(int ii=nallpoints-1; ii>nallpoints-5; --ii){
      if(fabs(yvalsall[ii]-plateauval)>5){
        plateauval=255;
	noPlateau=1;
        continue;
      }
    }
    
    int NbofPointsInPlateau=0;
    for(int ii=0; ii<nallpoints; ++ii)
      if(fabs(yvalsall[ii]-plateauval)<10 || yvalsall[ii]==0) NbofPointsInPlateau++;
    //summary_<<"row_"<<ipix->row()<<" col_"<<ipix->col()<<"   "<<plateauval<<"  "<<NbofPointsInPlateau<<"  "<<nallpoints<<endl;
    if(NbofPointsInPlateau>=(nallpoints-2)){
      status=-2;
      bookkeeper_2D_[detid]["status_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,status);
      if(writeSummary_){
        summary_<<"row_"<<ipix->row()<<" col_"<<ipix->col()<<" status_"<<status<<endl;
	statusNumbers_[abs(status)]++;
      }
      return false;
    }
  }
  else plateauval=255;
    
  double maxgoodvalinfit=plateauval*(1.-reject_badpoints_frac_);
  npoints=0;
  for(int ii=0; ii<nallpoints; ++ii){
   
    // now selecting the appropriate points for the fit.
    use_point=true;
    if(reject_first_point_ && xvalsall[ii]<0.1)
      use_point=false;
    if(ipix->getnentries(ii)<=min_nentries_ && reject_single_entries_)
      use_point=false;
    if(ipix->getnentries(ii)==0 && reject_badpoints_)
      use_point=false;
    if(yvalsall[ii]>maxgoodvalinfit && !noPlateau)
      use_point=false;
    if(ii>1 && fabs(yvalsall[ii]-yvalsall[ii-1])<5. && yvalsall[ii]>0.8*maxgoodvalinfit && reject_plateaupoints_){
      use_point=false;
      break;
    }
    
    if(use_point){
      xvals[npoints]=xvalsall[ii];
      yvals[npoints]=yvalsall[ii];
      yerrvals[npoints]=yerrvalsall[ii];
      npoints++;
    }
  }
  
  float chi2,slope,intercept,prob,slopeerror,intercepterror;
  prob=chi2=-1;
  slope=intercept=slopeerror=intercepterror=0;
  

  // now check on number of points. If bad just start taking the first 4:

  if(npoints<4){
    npoints=0;
    for(int ii=0; ii<nallpoints && npoints<4 && yvalsall[ii]<plateauval*0.97; ++ii){
      if(yvalsall[ii]>0){
	if(ii>0 && yvalsall[ii]-yvalsall[ii-1]<0.1)
	  continue;
	xvals[npoints]=xvalsall[ii];
	yvals[npoints]=yvalsall[ii];
	yerrvals[npoints]=yerrvalsall[ii];
	npoints++;
      }
    }
  }
  float binwidth = useVcalHigh_? scalarVcalHigh_VcalLow_ : 1;
  if(npoints<2){
    status = -7;
    bookkeeper_2D_[detid]["status_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,status);
    if(writeSummary_){
      summary_<<"row_"<<ipix->row()<<" col_"<<ipix->col()<<" status_"<<status<<endl;
      statusNumbers_[abs(status)]++;
    }
    std::ostringstream pixelinfo;
    pixelinfo << "GainCurve_row_" << ipix->row() << "_col_" << ipix->col();
    std::string tempname=translateDetIdToString(detid);
    tempname+="_";
    tempname+=pixelinfo.str();
    // setDQMDirectory(detid);

    //bookkeeper_pixels_[detid][pixelinfo.str()] = bookDQMHistogram1D(detid,pixelinfo.str(),tempname,105*nallpoints,xvalsall[0],xvalsall[nallpoints-1]*1.05);
    bookkeeper_pixels_1D_[detid][pixelinfo.str()] = bookHistogram1D(detid,pixelinfo.str(),tempname,(xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1,xvalsall[0]-binwidth/2.0,xvalsall[nallpoints-1]+binwidth/2.0 , currentDir );
    // std::cout  << "before" << std::endl;
    // std::cout  << "detid " << detid<< " pixelinfo.str() " << pixelinfo.str() << std::endl;
    // std::cout << "tempname " << tempname << " 105* nallpoints " << 105*nallpoints << " xvalsall[0] " <<  xvalsall[0] << " xvalsall[nallpoints-1]*1.05 " << xvalsall[nallpoints-1]*1.05 <<std::endl;
    // std::cout << "tempname " << tempname << " (xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1 " << (xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1 << " xvalsall[0]-binwidth/2.0 "<< xvalsall[0]-binwidth/2.0 << " xvalsall[nallpoints-1]+binwidth/2.0 " << xvalsall[nallpoints-1]+binwidth/2.0 << std::endl;

    for(int ii=0; ii<nallpoints; ++ii)
      bookkeeper_pixels_1D_[detid][pixelinfo.str()]->Fill(xvalsall[ii],yvalsall[ii]);
    return false;
  }
    
  //  std::cout << "starting fit!" << std::endl;
  graph_->Set(npoints);
  
  func_->SetParameter(0,50.);
  func_->SetParameter(1,0.25);
  for(int ipointtemp=0; ipointtemp<npoints; ++ipointtemp){
    graph_->SetPoint(ipointtemp,xvals[ipointtemp],yvals[ipointtemp]);
    graph_->SetPointError(ipointtemp,0,yerrvals[ipointtemp]);
  }
  Int_t tempresult = graph_->Fit("func","FQ0N");
  if (makehistopersistent) 
    {
      //std::cout << "swdebug: saving TGraph." << std::endl;
      TGraphErrors *savedGraph = bookTGraphs(detid, "savedGraph", npoints, xvals, yvals, 0, yerrvals, GetPixelDirectory(detid)) ;
    }
  slope=func_->GetParameter(1);
  slopeerror=func_->GetParError(1);
  intercept=func_->GetParameter(0);
  intercepterror=func_->GetParError(0);
  chi2=func_->GetChisquare()/((float)npoints-func_->GetNpar());
  prob= TMath::Prob(func_->GetChisquare(),npoints-func_->GetNpar());
  size_t ntimes=0;
  while((isnan(slope) || isnan(intercept) )&& ntimes<10){
    ntimes++;
    makehistopersistent=true;
    //    std::cout << slope << " " << intercept << " " << prob << std::endl;
    edm::LogWarning("SiPixelGainCalibrationAnalysis") << "impossible to fit values, try " << ntimes << ": " ;
    for(int ii=0; ii<npoints; ++ii){
      edm::LogWarning("SiPixelGainCalibrationAnalysis")<< "vcal " << xvals[ii] << " response: " << yvals[ii] << "+/-" << yerrvals[ii] << std::endl; 
    } 
    tempresult = graph_->Fit("func","FQ0NW");
    slope=func_->GetParameter(1);
    slopeerror=func_->GetParError(1);
    intercept = func_->GetParameter(0);
    intercepterror=func_->GetParError(0);
    chi2=func_->GetChisquare()/((float)npoints-func_->GetNpar());
    prob= TMath::Prob(func_->GetChisquare(),npoints-func_->GetNpar());
  }
  
  if(tempresult==0)
    status=1;
  else
    status=0;
  if(slope!=0)
    slope = 1./slope;
  if(isnan(slope) || isnan(intercept)){
    status=-6;
    bookkeeper_2D_[detid]["status_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,status);
    if(writeSummary_){
      summary_<<"row_"<<ipix->row()<<" col_"<<ipix->col()<<" status_"<<status<<endl;
      statusNumbers_[abs(status)]++;
    }
    //return false;
  }
  if(chi2>chi2Threshold_ && chi2Threshold_>=0)
    status=5;
  if(prob<chi2ProbThreshold_)
    status=5;
  if(noPlateau)
    status=3;
  if(nallpoints<4)
    status=-7;
  if(TMath::Abs(slope>maxGainInHist_) || slope < 0)
    status=-8;
  if(status!=1) makehistopersistent=true;
  statusNumbers_[abs(status)]++;


  if(slope<gainlow_)
    gainlow_=slope;
  if(slope>gainhi_)
    gainhi_=slope;
  if(intercept>pedhi_)
    pedhi_=intercept;
  if(intercept<pedlow_)
    pedlow_=intercept;
  bookkeeper_1D_[detid]["gain_1d"]->Fill(slope);
  if(slope>maxGainInHist_){
    makehistopersistent=true;
    edm::LogWarning("SiPixelGainCalibration") << "For DETID " << detid << "pixel row,col " << ipix->row() << "," << ipix->col() << " Gain was measured to be " << slope << " which is outside the range of the summary plot (" <<maxGainInHist_ << ") !!!! " << std::endl;
  }
  bookkeeper_2D_[detid]["dynamicrange_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,highmeanval-lowmeanval);
  bookkeeper_2D_[detid]["plateau_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,highmeanval);
  bookkeeper_2D_[detid]["gain_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,slope);
  bookkeeper_2D_[detid]["errorgain_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,slopeerror);
  bookkeeper_1D_[detid]["ped_1d"]->Fill(intercept);
  bookkeeper_2D_[detid]["ped_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,intercept);
  bookkeeper_2D_[detid]["errorped_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,intercepterror);
  bookkeeper_1D_[detid]["chi2_1d"]->Fill(chi2);
  bookkeeper_2D_[detid]["chi2_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,chi2);
  bookkeeper_1D_[detid]["prob_1d"]->Fill(prob);
  bookkeeper_2D_[detid]["prob_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,prob);
  bookkeeper_1D_[detid]["lowpoint_1d"]->Fill(xvals[0]);
  bookkeeper_2D_[detid]["lowpoint_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,xvals[0]);
  bookkeeper_1D_[detid]["highpoint_1d"]->Fill(xvals[npoints-1]);
  bookkeeper_2D_[detid]["highpoint_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,xvals[npoints-1]);
  bookkeeper_1D_[detid]["nfitpoints_1d"]->Fill(npoints);
  bookkeeper_1D_[detid]["endpoint_1d"]->Fill((255 - intercept)*slope);
  bookkeeper_2D_[detid]["status_2d"]->SetBinContent(ipix->col()+1,ipix->row()+1,status);

  if(!savePixelHists_)
    return true;
  if(detidfinder==listofdetids_.end() && listofdetids_.size()!=0)
    return true;
  if(makehistopersistent){
    std::ostringstream pixelinfo;
    pixelinfo << "GainCurve_row_" << ipix->row() << "_col_" << ipix->col();
    std::string tempname=translateDetIdToString(detid);
    tempname+="_";
    tempname+=pixelinfo.str();
    
    // and book the histo
    // fill the last value of the vcal array...   


    //bookkeeper_pixels_[detid][pixelinfo.str()] =  bookDQMHistogram1D(detid,pixelinfo.str(),tempname,105*nallpoints,xvalsall[0],xvalsall[nallpoints-1]*1.05);
    //TH1D* h = new TH1D("h","h",105*nallpoints,xvalsall[0],xvalsall[nallpoints-1]*1.05);
    //bookkeeper_pixels_[detid][pixelinfo.str()] =  bookDQMHistogram1D(detid,pixelinfo.str(),tempname,(xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1,xvalsall[0]-binwidth/2.0,xvalsall[nallpoints-1]+binwidth/2.0);
    //TH1D* h = new TH1D("h","h",(xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1,xvalsall[0]-binwidth/2.0,xvalsall[nallpoints-1]+binwidth/2.0);
 
    // std::cout  << "Later"<<std::endl;
    // std::cout  << "detid " << detid<< " pixelinfo.str() " << pixelinfo.str() << std::endl;
    // std::cout << "tempname " << tempname << " 105* nallpoints " << 105*nallpoints << " xvalsall[0] " <<  xvalsall[0] << " xvalsall[nallpoints-1]*1.05 " << xvalsall[nallpoints-1]*1.05 <<std::endl;
    // std::cout << "tempname " << tempname << " (xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1 " << (xvalsall[nallpoints-1]-xvalsall[0])/binwidth+1 << " xvalsall[0]-binwidth/2.0 "<< xvalsall[0]-binwidth/2.0 << " xvalsall[nallpoints-1]+binwidth/2.0 " << xvalsall[nallpoints-1]+binwidth/2.0 << std::endl;


    edm::LogInfo("SiPixelGainCalibrationAnalysis") << "now saving histogram for pixel " << tempname << ", gain = " << slope << ", pedestal = " << intercept << ", chi2/NDF=" << chi2 << "(prob:" << prob << "), fit status " << status;
    //for(int ii=0; ii<nallpoints; ++ii){
    //  //      std::cout << xvalsall[ii]<<","<<yvalsall[ii]<< " " << tempfloats[ii] << std::endl;
    //  bookkeeper_pixels_[detid][pixelinfo.str()]->Fill(xvalsall[ii],yvalsall[ii]);
    //  //std::cout<<"i: "<<ii<<" x: "<<xvalsall[ii]<<" center: "<<h->GetBinCenter(h->FindBin(xvalsall[ii]))<<std::endl;
    //}
    
    //    addTF1ToDQMMonitoringElement(bookkeeper_pixels_[detid][pixelinfo.str()],func_); ////adding func_ to the list of functions of the hist
    
    
    if(writeSummary_){
      summary_<<"row_"<<ipix->row()<<" col_"<<ipix->col();
      summary_<<" status_"<<status;
      summary_<<endl;
      
      //std::cout<<detid<<"  "<<"row " <<ipix->row()<<" col "<<ipix->col()<<"  "<<status<<"  "<<chi2<<"  "<<prob<<"  "<<npoints<<"  "<<xvals[0]<<"  "<<xvals[npoints-1]<<"  "<<plateauval<<std::endl;
    }
  } 
  return true;
}
// ------------ method called to do fill new detids  ------------
void 
SiPixelGainCalibrationAnalysis::newDetID(uint32_t detid)
{
  // setDQMDirectory(detid);
  std::string correntDir = GetPixelDirectory( detid// , myTFileDirMap
					      );
  std::string tempname=translateDetIdToString(detid);
  bookkeeper_1D_[detid]["gain_1d"] = bookHistogram1D(detid,"Gain1d","gain for "+tempname,100,0.,maxGainInHist_,correntDir);
  bookkeeper_2D_[detid]["gain_2d"] = bookHistoPlaquetteSummary2D(detid, "Gain2d","gain for "+tempname, correntDir);
  bookkeeper_2D_[detid]["errorgain_2d"] = bookHistoPlaquetteSummary2D(detid, "ErrorGain2d","error on gain for "+tempname, correntDir);
  bookkeeper_1D_[detid]["ped_1d"] = bookHistogram1D(detid,"Pedestal1d","pedestal for "+tempname,256,0.,256.0,correntDir);
  bookkeeper_2D_[detid]["ped_2d"] = bookHistoPlaquetteSummary2D(detid,"Pedestal2d","pedestal for "+tempname, correntDir);
  bookkeeper_2D_[detid]["errorped_2d"] = bookHistoPlaquetteSummary2D(detid,"ErrorPedestal2d","error on pedestal for "+tempname, correntDir);
  bookkeeper_1D_[detid]["chi2_1d"] = bookHistogram1D(detid,"GainChi2NDF1d","#chi^{2}/NDOF for "+tempname,100,0.,maxChi2InHist_,correntDir);
  bookkeeper_2D_[detid]["chi2_2d"] = bookHistoPlaquetteSummary2D(detid,"GainChi2NDF2d","#chi^{2}/NDOF for "+tempname, correntDir);
  bookkeeper_1D_[detid]["prob_1d"] = bookHistogram1D(detid,"GainChi2Prob1d","P(#chi^{2},NDOF) for "+tempname,100,0.,1.0,correntDir);
  bookkeeper_2D_[detid]["prob_2d"] = bookHistoPlaquetteSummary2D(detid,"GainChi2Prob2d","P(#chi^{2},NDOF) for "+tempname, correntDir);
  bookkeeper_2D_[detid]["status_2d"] = bookHistoPlaquetteSummary2D(detid,"GainFitResult2d","Fit result for "+tempname, correntDir);
  bookkeeper_1D_[detid]["endpoint_1d"]= bookHistogram1D(detid,"GainEndPoint1d","point where fit meets ADC=255 for "+tempname,256,0.,256.*scalarVcalHigh_VcalLow_,correntDir);
  bookkeeper_1D_[detid]["lowpoint_1d"]= bookHistogram1D(detid,"GainLowPoint1d","lowest fit point for "+tempname,256,0.,256.*scalarVcalHigh_VcalLow_,correntDir);
  bookkeeper_1D_[detid]["highpoint_1d"]= bookHistogram1D(detid,"GainHighPoint1d","highest fit point for "+tempname,256,0.,256.*scalarVcalHigh_VcalLow_,correntDir);
  bookkeeper_1D_[detid]["nfitpoints_1d"]= bookHistogram1D(detid,"GainNPoints1d","number of fit point for "+tempname,20,0.,20,correntDir);
  bookkeeper_2D_[detid]["dynamicrange_2d"]=bookHistoPlaquetteSummary2D(detid,"GainDynamicRange2d","Difference lowest and highest points on gain curve for "+tempname, correntDir); 
  bookkeeper_2D_[detid]["lowpoint_2d"]=bookHistoPlaquetteSummary2D(detid,"GainLowPoint2d","lowest fit point for "+tempname, correntDir);
  bookkeeper_2D_[detid]["highpoint_2d"]=bookHistoPlaquetteSummary2D(detid,"GainHighPoint2d","highest fit point for "+tempname, correntDir);
  bookkeeper_2D_[detid]["plateau_2d"]=bookHistoPlaquetteSummary2D(detid,"GainSaturate2d","Highest points on gain curve for "+tempname, correntDir);

}
//define this as a plug-in
DEFINE_FWK_MODULE(SiPixelGainCalibrationAnalysis);
