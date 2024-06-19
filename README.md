# Pixel gain calibration

Code to run the gain calibration and obtain pedestals (offsets) and gains (slopes) per pixel.

The gain calibration is explained in pages 23 to 24 in the [Phase-I pixel paper](https://arxiv.org/pdf/2012.14304.pdf).
An overview was presented in [these slides](https://indico.cern.ch/event/1011744/#1-pixel-gain-calibration).

Instructions for DOCs can be found on this TWiki: https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelGainCalibrationDoc

## Installation
Prepare your working directory with CMSSW
```
source /cvmfs/cms.cern.ch/cmsset_default.sh
export SCRAM_ARCH=el9_amd64_gcc11
cmsrel CMSSW_13_0_2
cd CMSSW_13_0_2/src
cmsenv
git clone https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration.git SiPixelTools/GainCalibration
git clone https://github.com/CMSTrackerDPG/SiPixelTools-PixelDumpDataInputSource.git SiPixelTools/PixelDumpDataInputSource
scram b -j 8
cd SiPixelTools/GainCalibration/test
```

## Local run
If you want to run the gain calibration on a single FED, please use [`test/gain_calib_cfg.py`](test/gain_calib_cfg.py)
For example, to run a test job, try
```
cp /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_10570/GainCalibration_1205_10570.dmp ./
cmsRun gain_calib_cfg.py run=10570 fed=1205
```
The output will be `GainCalibration.root`. Check `text_output.log` for errors.

## Submission
To launch the gain calibration process, you need to use [`test/run.py`](test/run.py) script,
which will submit one job for each FED to the HTCondor batch system.
First, prepare the job directory with
```
./run.py create RUNNUMBER -i INPUTDIR -o OUTPUTDIR
```
For example,
```
./run.py create 10570 -i /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_10570 -o /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/$USER/
```
To run from the default `Phase1/Run_*` directory, which is what you will do for any "real" gain calibration, simply do
```
./run.py create 10570
```
This creates a folder with a config file storing information about input/output folders
and creates the job scripts for the submission (from the template).
If you want to run over BPIX only, use the `--BPix-only` option
```
./run.py create 10570 --BPix-only
```

## Status & resubmission
Submit your jobs
```
./run.py submit 10570
```
You can check the status of your jobs with `condor_q`, or with
```
./run.py status 10570
```
If jobs fail, please use
```
./run.py resubmit 10570
```
To resubmit one or more specific FED jobs, please use `--fed`.

## Finalizing
Now, hadd the output
```
./run.py hadd 10570
```
This creates a large `GainCalibration.root` file.
Create a summary pdf with
```
./run.py summary 10570
```
Then create a tar file with the output and print some output to be posted at the [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelGainCalibrationDoc):
```
./run.py twiki 10570
```
The last step is to produce the payload (a database object)
```
./run.py payload 10570 YEAR VERSION
```
where YEAR is the year the calibration was taken and VERSION is the number of payloads produced in that year.
After this is done, please notify the following people of the location of the database object,
currently (this list might change):
Danek Kotlinksi <danek.kotlinskiATpsi.ch>,
Tamás Vámi <vami.tamasATwigner.mta.hu>,
Tanja Susa <Tatjana.SusaATcern.ch>.

## VCal database object
To update the VCal database object, please see the [TWiki](https://twiki.cern.ch/twiki/bin/view/CMS/SiPixelVCalHistory).
Basically:
```
export SCRAM_ARCH=el9_amd64_gcc11
cmsrel CMSSW_13_0_2
cd CMSSW_13_0_2/src
cmsenv
git cms-addpkg CondTools/SiPixel
scram b -j8
cd CondTools/SiPixel/test

# edit hardcoded VCal -> #electrons slopes/offsets in SiPixelVCalDB_cfg.py
cmsRun SiPixelVCalDB_cfg.py
```

## Contact
If you have issues with running the gain calibration code or `run.py`, please contact
Lars Noehte <lars.noehteATpsi.ch> and
Izaak Neutelings <izaak.neutelingsATuzh.ch>.

