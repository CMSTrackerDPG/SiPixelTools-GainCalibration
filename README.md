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
luminosity in between the `2024_v1` [elog](http://cmsonline.cern.ch/cms-elog/1219824) and `2025_v0` [elog](http://cmsonline.cern.ch/cms-elog/1253369) gain calibration
tags. This is best done in a separate terminal connected to lxplus. The specified time span does not have to be precise down to an hour, it is sufficient to get the
dates correctly. However, `brilcalc` tends to crash on longer time spans so it might be necessary to break it down into multiple time spans that actually contain
some luminosity. For example, the actual time span in this particular case should be `--begin "06/13/24 00:00:00" --end "03/25/25 00:00:00` but due to `brilcalc`
crashing, it has been shortened as shown below
```
source /cvmfs/cms-bril.cern.ch/cms-lumi-pog/brilws-docker/brilws-env

brilcalc lumi -u /fb --normtag /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_BRIL.json --begin "04/02/24 00:00:00" --end "11/04/24 00:00:00"
```
The obtained amount of delivered integrated luminosity in this case is 123.280322777 /fb. This amount has been added to
[`test/calc_corrections.py`](test/calc_corrections.py) with the following line
```
lumi["2025_v0"] = lumi["2024_v1"] + 123.280322777  # in fb^{-1}
```
Next, we need to obtain the updated correction factors for VCal -> #electrons slopes and put them in [`test/SiPixelVCalDB_cfg.py`](test/SiPixelVCalDB_cfg.py). This is done by running
```
python3 calc_corrections.py
```
which prints out the correction factors to the screen
```
values used for 2025_v0 new model:
corrs_bpix = {1: 1.095, 2: 1.056, 3: 1.039, 4: 1.023}
corrs_fpix = {1: 1.1397, 2: 1.0914}
```
and also produces an output file named `calc_corrections_2025_v0.png` that contain a graph of received doses (in Mrad) for different parts of the pixel detector.

To get the updated database object, run
```
cmsRun SiPixelVCalDB_cfg.py
```
This produces the output file named `SiPixelVCal_phase1_2025_v0.db`. After producing the file, please update the corresponding [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelVCalHistory).


## Local run
If you want to run the gain calibration on a single FED, please use [`test/gain_calib_cfg.py`](test/gain_calib_cfg.py)
For example, to run a test job, try
```
cp /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_10901/GainCalibration_1205_10901.dmp ./
cmsRun gain_calib_cfg.py run=10901 fed=1205
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
./run.py create 10901 -t SiPixelVCal_phase1_2025_v0 -i /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_10901 -o /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/$USER/
```
To run from the default `Phase1/Run_*` directory, which is what you will do for any "real" gain calibration, simply do
```
./run.py create 10901 -t SiPixelVCal_phase1_2025_v0
```
This creates a folder with a config file storing information about input/output folders
and creates the job scripts for the submission (from the template).
If you want to run over BPIX only, use the `--BPix-only` option
```
./run.py create 10901 -t SiPixelVCal_phase1_2025_v0 --BPix-only
```

## Status & resubmission
Submit your jobs
```
./run.py submit 10901
```
You can check the status of your jobs with `condor_q`, or with
```
./run.py status 10901
```
If jobs fail, please use
```
./run.py resubmit 10901
```
To resubmit one or more specific FED jobs, please use `--fed`.


## Finalizing
Now, hadd the output
```
./run.py hadd 10901
```
This creates a large `GainCalibration.root` file.

Create a summary pdf with
```
./run.py summary 10901
```
Then create a tar file with the output and print some output
<!-- to be posted on the [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelGainCalibrationDoc) but is no longer actively done. -->
```
./run.py twiki 10901
```
The last step is to produce the payloads (database objects) for offline and the HLT
```
./run.py payload 10901 YEAR VERSION
```
where YEAR is the year the calibration was taken and VERSION is the number of payloads produced in that year, for example
```
./run.py payload 10901 2025 0
```
After producing the payloads, please update the corresponding [TWiki](https://twiki.cern.ch/twiki/bin/viewauth/CMS/SiPixelGainHistory).

## Contact
If you have issues with running the gain calibration code or `run.py`, please contact
Dinko Ferencek <Dinko.FerencekATcern.ch>.

