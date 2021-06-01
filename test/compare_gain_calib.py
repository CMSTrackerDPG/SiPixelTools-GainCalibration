#! /usr/bin/env python
# Author: Izaak Neutelings (May 2020)
# Description: Compare two gain calibration files for debugging
#   cp /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_323203/GainCalibration_1205_323203.dmp ./
#   cmsRun gain_calib_cfg.py run=323203 fed=1205 minPVal=0.05
#   cp /eos/cms/store/group/dpg_tracker_pixel/comm_pixel/GainCalibrations/Phase1/Run_323203/GainRun_323203/1205.root ./
#   ./compare_gain_calib.py 1205.root GainCalibration.root
import os, sys
from math import log10
import fnmatch # for glob wildcard '*'
import ROOT; ROOT.PyConfig.IgnoreCommandLineOptions = True
from ROOT import gROOT, gStyle, TFile, TCanvas, TLegend, TH2, \
                 kBlue, kRed, kGreen, kMagenta, kDashed
gROOT.SetBatch(True)       # don't open GUI windows
gStyle.SetOptTitle(False)  # don't make title on top of histogram
gStyle.SetLineStyleString(kDashed,"30 30"); # make kDashed wider
#gStyle.SetOptStat(False)    # don't make stat. box


def scalevec(a,b,r,log=False):
  """Scale vector ab by r."""
  if log:
    assert a!=0, "scale: Cannot logarithmically scale ab vector if a=0!"
    assert a*b>0, "scale: Cannot logarithmically scale ab vector if a and b do not have the same sign! a=%s, b=%s"%(a,b)
    span = abs(r*log10(b/a)) # get magnitude range
    return a*(10**span)
  return a+r*(b-a)
  

def ensureTFile(fname,option='READ',verb=0):
  """Open TFile, checking if the file in the given path exists."""
  if isinstance(fname,basestring):
    if not os.path.isfile(fname):
      raise IOError('File in path "%s" does not exist!'%(fname))
      exit(1)
    file = ROOT.TFile.Open(fname,option)
    if not file or file.IsZombie():
      raise IOError('Could not open file by name %r!'%(fname))
    if verb>=1:
      print "Opened file %s..."%(fname)
  else:
    file = fname
    if not file or (hasattr(file,'IsZombie') and file.IsZombie()):
      raise IOError('Could not open file %r!'%(file))
  return file
  


def ensuredir(dirname,**kwargs):
  """Make directory if it does not exist."""
  verbosity = kwargs.get('verb',  0    )
  if not dirname:
    pass
  elif not os.path.exists(dirname):
    os.makedirs(dirname)
    if verbosity>=1:
      print '>>> Made directory "%s"'%(dirname)
    if not os.path.exists(dirname):
      print '>>> Failed to make directory "%s"'%(dirname)
  return dirname
  

def getdirs(file,dirpath,depth=0,verb=0):
  """Get TDirectories recursively."""
  dirnames = dirpath.strip('/').split('/')
  cwdpath = "/" # root
  dirs = [ ] # return value
  icwd = 0
  prefix = ">>> getdirs: "+' '*depth
  for subdir in dirnames:
    if '*' in subdir:
      break # skip to top
    icwd += 1
  if icwd>0:
    cwdpath = '/'.join(dirnames[:icwd]) # cwd
    dirpath = '/'.join(dirnames[icwd:]) # new dir path
    if verb>=2:
      print prefix+"Skip to cwdpath=%r, look for dirpath=%r"%(cwdpath,dirpath)
    cwddir = file.Get(cwdpath)
    assert cwddir, "Could not open TDirectory %r in %s"%(cwdpath,file.GetPath())
  else:
    cwddir = file
  if not dirpath:
    return [cwddir]
  searchpath = dirnames[icwd] # look for this TDirectory in cwddir
  nextpath = '/'.join(dirnames[icwd+1:]) if icwd<len(dirnames) else "" # next dir path
  if verb>=2:
    print prefix+"cwdpath=%r, dirpath=%r"%(cwdpath,dirpath)
    print prefix+"searchpath=%r, nextpath=%r"%(searchpath,nextpath)
  for key in cwddir.GetListOfKeys():
    if not gROOT.GetClass(key.GetClassName()).InheritsFrom('TDirectory'): continue
    #print cwddir.Get(key.GetName()), key, key.GetClassName(), type(key)
    dir = cwddir.Get(key.GetName())
    dirname = dir.GetName()
    match = fnmatch.fnmatch(dirname,searchpath)
    if verb>=3:
      print prefix+"Match %r to %r in %s: %r"%(dirname,searchpath,cwddir.GetPath(),match)
    if not match: continue # ignore
    if nextpath: # look for path of dirpath recursively
      dirs.extend(getdirs(dir,nextpath,depth=depth+2,verb=verb))
    else: # stop recursion
      if verb>=3:
        print prefix+"  Stop recursion" #with %r in %s"%(dirname,file.GetPath())
      dirs.append(dir)
  if depth==0:
    assert dirs, "Could not find %r in %s"%(dirpath,file.GetPath())
    if verb>=1:
      print prefix+"Found: "
      for dir in dirs:
        print prefix+"  %s (%r)"%(dir.GetPath(),dir)
  return dirs
  

def gethists(file,hnames,verb=0):
  """Get histograms recursively."""
  dirs = { }
  hists = { }
  if isinstance(hnames,str):
    hnames = [hnames]
  for fullsearchhname in hnames:
    dirname = os.path.dirname(fullsearchhname)
    searchhname = os.path.basename(fullsearchhname)
    nfound = 0
    if verb>=1:
      print ">>> gethists: Look for %s in %s"%(searchhname,dirname)
    if dirname not in dirs:
      dirs[dirname] = getdirs(file,dirname,verb=verb) # cache for later reuse
    for dir in dirs[dirname]:
      for key in dir.GetListOfKeys():
        if not gROOT.GetClass(key.GetClassName()).InheritsFrom('TH1'): continue
        #print dir.Get(key.GetName()), key, key.GetClassName(), type(key)
        hist = dir.Get(key.GetName())
        hname = hist.GetName()
        match = fnmatch.fnmatch(hname,searchhname)
        if verb>=3 or (match and verb>=2):
          print ">>> gethists: Match %r to %r in %s: %r"%(hname,searchhname,file.GetPath(),match)
        if not match: continue
        fullhname = dir.GetPath().split(':/')[-1]+'/'+hname
        if fullhname not in hists:
          hists[fullhname] = hist
          nfound += 1
        else:
          print ">>> gethists: Warning! Found %r twice: %s and %s"%(fullhname,hists[fullhname],hist)
    assert nfound>0, "gethists: Warning! Could not find %r in %s"%(searchhname,file.GetPath())
  if verb>=1:
    print ">>> gethists: Found: "
    for hname, hist in hists.iteritems():
      print ">>> gethists:   %s (%r)"%(hname,hist)
  return hists
  

def draw(pname,hists,**kwargs):
  """Compare 1D histograms."""
  tag     = kwargs.get('tag',    ""    )
  xtitle  = kwargs.get('xtitle', None  )
  ytitle  = kwargs.get('ytitle', None  )
  logx    = kwargs.get('logx',   False )
  logy    = kwargs.get('logy',   False )
  header  = kwargs.get('header', ""    )
  norm    = kwargs.get('norm',   False )
  dividebybins = kwargs.get('dividebybins', False )
  gStyle.SetOptStat('nemruo') # https://root.cern.ch/doc/master/classTStyle.html#a906e5f9060357a95f893701b3bed57a2
  hmargin = 2.0 if logy else 1.15
  tsize   = 0.045
  colors  = [ kBlue, kRed, kGreen+2, kMagenta ]
  styles  = [ 1, 2 ]
  lmargin, rmargin = 0.11, 0.04
  bmargin, tmargin = 0.12, 0.06
  width   = 0.45
  if norm:
    for hist in hists:
      hint = hist.Integral()
      if hint<=0: continue
      hist.Scale(1./hint)
  if dividebybins:
    for hist in hists:
      for i in range(1,hist.GetXaxis().GetNbins()+2):
        bwidth = hist.GetXaxis().GetBinWidth(i)
        hist.SetBinContent(i,hist.GetBinContent(i)/bwidth)
        hist.SetBinError(i,hist.GetBinError(i)/bwidth)
  xmin = min(h.GetXaxis().GetXmin() for h in hists)
  xmax = max(h.GetXaxis().GetXmax() for h in hists)
  ymin = min(h.GetMinimum() for h in hists)
  ymax = max(h.GetMaximum() for h in hists)
  
  # CANVAS
  canvas = TCanvas('canvas','canvas',100,100,900,800)
  canvas.SetMargin(lmargin,rmargin,bmargin,tmargin) # LRBT
  if logx:
    canvas.SetLogx()
    if xmin==0:
      xmin = 5e-1 if xmax>1e2 else 1e-3
  if logy:
    canvas.SetLogy()
    if ymin==0:
      ymin = 5e-1 if ymax>1e2 else 1e-3
  ymax = scalevec(ymin,ymax,1.15,log=logy)
  frame = canvas.DrawFrame(xmin,ymin,xmax,ymax)
  #frame = hists[0]
  if xtitle!=None:
    frame.GetXaxis().SetTitle(xtitle)
  if ytitle!=None:
    frame.GetYaxis().SetTitle(ytitle)
  frame.GetXaxis().SetTitleSize(tsize)
  frame.GetYaxis().SetTitleSize(tsize)
  frame.GetXaxis().SetLabelSize(0.9*tsize)
  frame.GetYaxis().SetLabelSize(0.9*tsize)
  frame.GetXaxis().SetTitleOffset(1.05)
  frame.GetYaxis().SetTitleOffset(1.04)
  #frame.SetMinimum(0)
  #frame.SetMaximum()
  #frame.GetXaxis().SetRangeUser(xmin,xmax)
  #frame.Draw('AXIS')
  
  # LEGEND
  height = tsize*1.1*len(hists)
  x1 = 0.40; x2 = x1 + width
  if header:
    height += tsize*1.15
  y1 = 0.92; y2 = y1 - height
  legend = TLegend(x1,y1,x2,y2)
  legend.SetFillStyle(0)
  legend.SetBorderSize(0)
  legend.SetTextSize(0.80*tsize)
  legend.SetMargin(0.3*width)
  legend.SetTextFont(42)
  
  # DRAW
  for i, hist in enumerate(hists):
    color = colors[i%len(colors)]
    style = styles[i%len(styles)]
    hist.SetLineColor(color)
    hist.SetLineWidth(2)
    hist.SetLineStyle(style)
    hist.SetMarkerSize(0.9)
    hist.SetMarkerColor(color)
    hist.SetMarkerStyle(8)
    hist.Draw('SAMES')
    legend.AddEntry(hist,hist.GetTitle(),'lp')
  legend.Draw()
  
  # BOXES
  boxes = [ ]
  canvas.Update()
  x1, y1 = 0.99, 0.99
  bwidth, bheight = 0.22, 0.13
  for i, hist in enumerate(hists):
    box = hist.FindObject('stats')
    #print ytop, bheight
    #print box.GetX1NDC(), box.GetX2NDC(), box.GetY1NDC(), box.GetY2NDC()
    #if i==0:
    #  bheight = abs(box.GetY2NDC()-box.GetY1NDC())
    box.SetY1NDC(y1)
    box.SetY2NDC(y1-bheight)
    box.SetX1NDC(x1)
    box.SetX2NDC(x1-bwidth)
    box.SetLineColor(hist.GetLineColor())
    box.SetTextColor(hist.GetLineColor())
    y1 -= bheight + 0.01
    boxes.append(box)
  
  canvas.SaveAs(pname+tag+".png")
  canvas.SaveAs(pname+tag+".pdf")
  canvas.Close()
  

def draw2d(pname,hist,xtitle=None,ytitle=None,ztitle=None,logz=False):
  """Draw 2D histogram."""
  gStyle.SetOptStat(False)
  tsize = 0.045
  xmin, xmax = hist.GetXaxis().GetXmin(), hist.GetXaxis().GetXmax()
  ymin, ymax = hist.GetYaxis().GetXmin(), hist.GetYaxis().GetXmax()
  canvas = TCanvas('canvas','canvas',100,100,900,800)
  canvas.SetMargin(0.11,0.16,0.10,0.02) # LRBT
  if logz:
    canvas.SetLogz()
  if xtitle!=None:
    hist.GetXaxis().SetTitle(xtitle)
  if ytitle!=None:
    hist.GetYaxis().SetTitle(ytitle)
  if ztitle!=None:
    hist.GetZaxis().SetTitle(ztitle)
  hist.GetXaxis().SetTitleSize(tsize)
  hist.GetYaxis().SetTitleSize(tsize)
  hist.GetZaxis().SetTitleSize(tsize)
  hist.GetXaxis().SetLabelSize(0.9*tsize)
  hist.GetYaxis().SetLabelSize(0.9*tsize)
  hist.GetZaxis().SetLabelSize(0.9*tsize)
  hist.GetXaxis().SetTitleOffset(1.04)
  hist.GetYaxis().SetTitleOffset(1.14)
  hist.GetZaxis().SetTitleOffset(1.08)
  hist.Draw('COLZ')
  canvas.RedrawAxis()
  canvas.SaveAs(pname+".png")
  canvas.SaveAs(pname+".pdf")
  canvas.Close()
  

def countstatus(hist,statuses={ }):
  """Compare status."""
  for ix in range(1,hist.GetXaxis().GetNbins()+1):
    for iy in range(1,hist.GetYaxis().GetNbins()+1):
      status = hist.GetBinContent(ix,iy)
      if status in statuses:
        statuses[status] += 1
      else:
        statuses[status] = 1
  return statuses
  

def compare(fname1,fname2,hnames,outdir="",verb=0):
  """Compare histograms."""
  title1, title2 = "Old", "New"
  if fname1.count('=')==1:
    title1, fname1 = fname1.split('=')
  if fname2.count('=')==1:
    title2, fname2 = fname2.split('=')
  file1 = ensureTFile(fname1)
  file2 = ensureTFile(fname2)
  statuses1, statuses2 = { }, { }
  ensuredir(outdir)
  #print getdirs(file1,"siPixelGainCalibrationAnalysis/Pixel/Barrel/*/*/*/*",verb=2)
  for searchhname in hnames:
    hists1 = gethists(file1,searchhname,verb=verb)
    hists2 = gethists(file2,searchhname,verb=verb)
    for filea, hista, fileb, histb in [(file1,hists1,file2,hists2),(file2,hists2,file1,hists1)]:
      for hname in hista:
        if hname not in histb:
          print ">>> compare: Warning! Could not find %s in %s"%(hname,fileb.GetPath())
    for fullhname, hist1 in hists1.iteritems():
      if fullhname not in hists2: continue
      hist2 = hists2[fullhname]
      pname = os.path.join(outdir,'_'.join(p.replace('_','') for p in fullhname.split('/')[2:]))
      pname = pname.replace("siPixelCalibDigis","_")
      if isinstance(hist1,TH2): # compare 2D
        pname1 = "%s_%s"%(pname,title1.replace(' ',''))
        pname2 = "%s_%s"%(pname,title2.replace(' ',''))
        if "FitResult" in fullhname:
          ztitle = "Status"
          logz = False
          statuses1 = countstatus(hist1,statuses1)
          statuses2 = countstatus(hist2,statuses2)
        else:
          ztitle = None
          logz = True
        draw2d(pname1,hist1,ztitle=ztitle,logz=logz)
        draw2d(pname2,hist2,ztitle=ztitle,logz=logz)
      else: # compare 1D
        xtitle = hist1.GetTitle()
        ytitle = "Pixels"
        hist1.SetTitle(title1)
        hist2.SetTitle(title2)
        logx = False #"GainChi2Prob1d" in fullhname
        draw(pname,[hist1,hist2],xtitle=xtitle,ytitle=ytitle,logx=logx,logy=True,norm=False)
        if "GainChi2NDF1d" in fullhname:
          draw(pname,[hist1,hist2],xtitle=xtitle,ytitle=ytitle,logx=logx,logy=True,norm=False,dividebybins=True,tag="_dividebybins")
  if statuses1 and statuses2:
    print ">>> Compare status of fit result:"
    ntot1 = sum(v for k,v in statuses1.iteritems())
    ntot2 = sum(v for k,v in statuses2.iteritems())
    print ">>> Total number of pixels: %d (%s), %d (%s)"%(ntot1,title1,ntot2,title2)
    print ">>> %8s %16s %16s"%("Status",title1,title2)
    for status in sorted(range(-11,11),key=lambda s: abs(s)):
      if status not in statuses1 and status not in statuses1: continue
      npix1 = statuses1.get(status,0)
      npix2 = statuses2.get(status,0)
      frac1 = 100.*npix1/ntot1
      frac2 = 100.*npix2/ntot2
      print ">>> %8s %8d %7.2f%% %8d %6.2f%%"%(status,npix1,frac1,npix2,frac2)
  

def main(args):
  files     = args.files
  outdir    = args.outdir
  verbosity = args.verbosity
  hdirs = "siPixelGainCalibrationAnalysis/Pixel/Barrel/*/*/*/*" # histogram directories
  #hdirs = "siPixelGainCalibrationAnalysis/Pixel/Barrel/Shell_pI/*/*/*"
  #hdirs = "siPixelGainCalibrationAnalysis/Pixel/Barrel/Shell_pI/Layer_1/Ladder_02F/Module_1"
  hists = [
    hdirs+"/Gain1d_*",
    hdirs+"/Pedestal1d_*",
    hdirs+"/GainChi2NDF1d_*",
    hdirs+"/GainChi2Prob1d_*",
    hdirs+"/GainEndPoint1d_*",
    hdirs+"/GainLowPoint1d_*",
    hdirs+"/GainHighPoint1d_*",
    hdirs+"/GainNPoints1d_*",
    hdirs+"/GainFitResult2d_*",
    #hdirs+"/Gain2d_*",
    #hdirs+"/ErrorGain2d_*",
    #hdirs+"/Pedestal2d_*",
    #hdirs+"/ErrorPedestal2d_*",
    #hdirs+"/GainChi2NDF2d_*",
    #hdirs+"/GainChi2Prob2d_*",
    #hdirs+"/GainDynamicRange2d_*",
    #hdirs+"/GainLowPoint2d_*",
    #hdirs+"/GainHighPoint2d_*",
    #hdirs+"/GainSaturate2d_*",
  ]
  compare(files[0],files[1],hists,outdir=outdir,verb=verbosity)
  

if __name__ == "__main__":
  from argparse import ArgumentParser
  argv = sys.argv
  description = """Compare gain calibration files."""
  parser = ArgumentParser(prog="compare_gain_calib",description=description,epilog="Good luck!")
  parser.add_argument('files',                nargs=2, action='store',
                                              help="two gain calibration files to compare; use optional TITLE=FILE format to set legend entries" )
  parser.add_argument('-o', '--outdir',       default="compare", action='store',
                                              help="output directory, default=%(default)r" )
  parser.add_argument('-v', '--verbose',      dest='verbosity', type=int, nargs='?', const=1, default=0, action='store',
                                              help="set verbosity, default=%(default)d, const=%(const)d" )
  args = parser.parse_args()
  main(args)
  print ">>>\n>>> Done."