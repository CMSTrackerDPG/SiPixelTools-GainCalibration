# Payload procedure for 2021 v2 created from CRUZET Run_14

Follow these two installation procedures:

* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration#installation
* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration#vcal-database-object

### Create new VCal sqlite object
change slope and offset for layer 1 in `CondTools/SiPixel/test/SiPixelVCalDB_cfg.py`
```
# values for VCal to n-electrons from 2018 -------------|  # modified for new, non-irradiated L1, end of 2021. l2,3,4 & fpix like end of 2018.            
slope      = 47.                                        |  slope      = 47.
slope_L1   = 50.                                        |  slope_L1   = 45.7                                                                              
offset     = -60.                                       |  offset     = -60.
offset_L1  = -670.                                      |  offset_L1  = -308.                                                                             
corrs_bpix = { 1: 1.110, 2: 1.036, 3: 1.023, 4: 1.011 } |  corrs_bpix = { 1: 1.000, 2: 1.036, 3: 1.023, 4: 1.011 } 
```

Run the script: `cmsRun SiPixelVCalDB_cfg.py`

Use `CondTools/SiPixel/test/SiPixelVCalReader_cfg.py` to create a root file, displaying the sqlite file content.

Copy the `siPixelVCal.db` to `CMSSW_11_3_0/src/SiPixelTools/GainCalibration/test/`.

### Before running the gain calibration analysis
#### Change the thresholds for the chi2 and p-value tests:
https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/gain_calib_cfg.py#L13-L14

Current default "disables" these two tests by setting the thresholds such that nearly all fits are included:
```python
options.register('minPVal', 0.0,   mytype=VarParsing.varType.float) # minChi2Prob, 0.0 to switch off
options.register('minChi2', 500.,    mytype=VarParsing.varType.float) # minChi2
```

#### Change the VCal tag to the one you like to use
define the tag name from the data base here:

* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/gain_calib_cfg.py#L173

or switch to using the local file:

* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/gain_calib_cfg.py#L167

#### Check the era and global tag
* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/gain_calib_cfg.py#L22

* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/gain_calib_cfg.py#L23

* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/SiPixelGainCalibrationDBUploader_cfg.py#L18

* https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/SiPixelGainCalibrationDBUploader_cfg.py#L71


#### Copy the .dmp files to the phase 1 directory
`/eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_14` for this CRUZET Run 14

### Run the gain calibration analysis and do the payload
Follow the instructions in the README.md

https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/master/README.md

For the summary plots, uncomment this line:
```python
#execute('root -l -b -x make_ComparisonPlots.cc+"(\\"%s\\",\\"%s\\")" -q'%(gainfile,run),verb=verbosity+1)
```
https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/run.py#L494


For the payloads, uncomment the payloads you would like to create: (full not needed)
```python
# OFFLINE PAYLOAD
cmd = ("cmsRun SiPixelGainCalibrationDBUploader_cfg.py"
       " run=%s year=%s dbversion=%s gain=$GAIN file=%s outdir=%s"%(run,year,dbversion,gainfile,paydir))
print "Creating payload for offline..."
execute(cmd.replace('$GAIN','offline'),verb=verbosity+1)

## OFFLINE FULL PAYLOAD
#print "Creating payload for offline full..."
#execute(cmd.replace('$GAIN','full'))

## HLT PAYLOAD
#print "Creating payload for HLT..."
#execute(cmd.replace('$GAIN','hlt'))
```
https://github.com/CMSTrackerDPG/SiPixelTools-GainCalibration/blob/b00c2806da7350ee64a38af63a45069682f09f75/test/run.py#L566-L578