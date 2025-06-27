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

## VCal database object
If there has been a significant amount of integrated luminosity delivered since the last update of the gain calibrations, the VCal database object needs to be updated
to take that into account. To determine the amount, the `brilcalc` tool can be used as in the following example that checks the amount of delivered integrated
luminosity in between the `2025_v0` [elog](http://cmsonline.cern.ch/cms-elog/1253369) and `2025_v1` [elog](http://cmsonline.cern.ch/cms-elog/1269682) gain calibration
tags. This is best done in a separate terminal connected to lxplus:
```
source /cvmfs/cms-bril.cern.ch/cms-lumi-pog/brilws-docker/brilws-env

brilcalc lumi -u /fb --normtag /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_BRIL.json --begin "03/25/25 00:00:00" --end "06/26/25 23:59:59"
```
The specified time span does not have to be precise down to an hour, it is sufficient to get the dates correctly. However, `brilcalc` has been observed to crash on
longer time spans and in those cases it was necessary to break it down into multiple time spans that actually contain some luminosity.
The obtained amount of delivered integrated luminosity in this particular case is 23.851410627 /fb. This amount has been added to
[`test/calc_corrections.py`](test/calc_corrections.py) with the following line
```
lumi["2025_v1"] = lumi["2025_v0"] + 23.851410627  # in fb^{-1}
```
Next, we need to obtain the updated correction factors for VCal -> #electrons slopes and put them in [`test/SiPixelVCalDB_cfg.py`](test/SiPixelVCalDB_cfg.py). This is done by running
```
python3 calc_corrections.py
```
which prints out the correction factors to the screen
```
values used for 2025_v1 new model:
corrs_bpix = {1: 1.095, 2: 1.055, 3: 1.038, 4: 1.023}
corrs_fpix = {1: 1.1393, 2: 1.0908}
```
and also produces an output file named `calc_corrections_2025_v1.png` that contain a graph of received doses (in Mrad) for different parts of the pixel detector.

To get the updated database object, run
```
cmsRun SiPixelVCalDB_cfg.py tagName=SiPixelVCal_phase1_2025_v1
```
This produces the output file named `SiPixelVCal_phase1_2025_v1.db`. After producing the file, please update the corresponding [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelVCalHistory).


## Getting input files

The pixel operations team takes a special local run that produces data files needed for deriving the gain calibrations. Once the run is taken, the data files are copied to a designated folder on the CERN EOS at the following path `/eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_#` where `#` is the run number of the local run. The output files from the analysis by default go into `/eos/cms/store/group/dpg_tracker_pixel/comm_pixel/$USER/`. To have write permissions to the `comm_pixel` area, one needs to be added to the [cms-eos-dpg-tracker](https://e-groups.cern.ch/e-groups/Egroup.do?egroupName=cms-eos-dpg-tracker) e-group as described [here](https://twiki.cern.ch/twiki/bin/view/CMS/T2CHCERNEosTeams).


## Local run
If you want to run the gain calibration on a single FED, please use [`test/gain_calib_cfg.py`](test/gain_calib_cfg.py)
For example, to run a test job, try
```
cp /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_10981/GainCalibration_1205_10981.dmp ./
cmsRun gain_calib_cfg.py vcalTag=SiPixelVCal_phase1_2025_v1 run=10981 fed=1205
```
The output will be `GainCalibration.root`. Check `text_output.log` for errors.


## Submission
To launch the gain calibration process, you need to use [`test/run.py`](test/run.py) script,
which will submit one job for each FED to the HTCondor batch system.
First, prepare the job directory with
```
./run.py create RUNNUMBER -t VCALTAG -i INPUTDIR -o OUTPUTDIR
```
For example,
```
./run.py create 10981 -t SiPixelVCal_phase1_2025_v1 -i /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_10981 -o /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/$USER/
```
To run from the default `Phase1/Run_*` directory, which is what you will do for any "real" gain calibration, simply do
```
./run.py create 10981 -t SiPixelVCal_phase1_2025_v1
```
This creates a folder with a config file storing information about input/output folders
and creates the job scripts for the submission (from the template).
If you want to run over BPIX only, use the `--BPix-only` option
```
./run.py create 10981 -t SiPixelVCal_phase1_2025_v1 --BPix-only
```

## Status & resubmission
Submit your jobs
```
./run.py submit 10981
```
You can check the status of your jobs with `condor_q`, or with
```
./run.py status 10981
```
If jobs fail, please use
```
./run.py resubmit 10981
```
To resubmit one or more specific FED jobs, please use `--fed`.


## Finalizing
Now, hadd the output
```
./run.py hadd 10981
```
This creates a large `GainCalibration.root` file.

Create a summary pdf with
```
./run.py summary 10981
```
Then create a tar file with the output and print some output
<!-- to be posted on the [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelGainCalibrationDoc) but is no longer actively done. -->
```
./run.py twiki 10981
```
The last step is to produce the payloads (database objects) for offline and the HLT
```
./run.py payload RUNNUMBER YEAR VERSION
```
where YEAR is the year the calibration was taken and VERSION is the number of payloads produced in that year, for example
```
./run.py payload 10981 2025 1
```
After producing the payloads, please update the corresponding [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelGainHistory). Also inform the AlCaDB
contact at cms-pixel-db-contactATcern.ch about the location of newly produced VCal and gain calibration payloads and the contact will upload them to
the conditions database.


## Payload Inspector plots

To get some visual information about the content of newly produced gain calibrations, the Payload Inspector tool can be used to produce some comparison plots. One option is to use a web-based interface available from the [CondDB browser](https://cms-conddb.cern.ch/cmsDbBrowser), which is not always very reliable, or, alternatively, one can produce the same plots at the command line using the following commands
```
getPayloadData.py --plugin pluginSiPixelGainCalibrationOffline_PayloadInspector --plot plot_SiPixelGainCalibOfflineGainComparisonBarrelTwoTags --tag SiPixelGainCalibration_2025_v0 --tagtwo SiPixelGainCalibration_2025_v1 --time_type Run --iovs '{"start_iov": "1", "end_iov": "1"}' --iovstwo '{"start_iov": "1", "end_iov": "1"}' --db Prod --test

getPayloadData.py --plugin pluginSiPixelGainCalibrationOffline_PayloadInspector --plot plot_SiPixelGainCalibOfflinePedestalComparisonBarrelTwoTags --tag SiPixelGainCalibration_2025_v0 --tagtwo SiPixelGainCalibration_2025_v1 --time_type Run --iovs '{"start_iov": "1", "end_iov": "1"}' --iovstwo '{"start_iov": "1", "end_iov": "1"}' --db Prod --test

getPayloadData.py --plugin pluginSiPixelGainCalibrationOffline_PayloadInspector --plot plot_SiPixelGainCalibOfflineGainComparisonEndcapTwoTags --tag SiPixelGainCalibration_2025_v0 --tagtwo SiPixelGainCalibration_2025_v1 --time_type Run --iovs '{"start_iov": "1", "end_iov": "1"}' --iovstwo '{"start_iov": "1", "end_iov": "1"}' --db Prod --test

getPayloadData.py --plugin pluginSiPixelGainCalibrationOffline_PayloadInspector --plot plot_SiPixelGainCalibOfflinePedestalComparisonEndcapTwoTags --tag SiPixelGainCalibration_2025_v0 --tagtwo SiPixelGainCalibration_2025_v1 --time_type Run --iovs '{"start_iov": "1", "end_iov": "1"}' --iovstwo '{"start_iov": "1", "end_iov": "1"}' --db Prod --test
```
The above plots compare gains and pedestals in BPix and FPix between the `2025_v0` and `2025_v1` gain calibrations.


## Contact
If you have issues with running the gain calibration code or `run.py`, please contact
Dinko Ferencek <Dinko.FerencekATcern.ch>.

