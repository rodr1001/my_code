import csv
import ROOT
from tqdm import tqdm

ROOT.gROOT.SetBatch(1)
f = ROOT.TFile('hist-no-filtering.root')
d = f.Get('MAC2')
fl = ROOT.TF1('seedparamslandau','landau')
fl.SetParameters(1,0.1,0.1)
fl.SetParLimits(1,0,5)
max_uv = 24
max_layer = 34

with open ('cellstats.csv', 'w') as file, tqdm(total=max_uv*max_uv*max_layer) as t:
    writer = csv.writer(file)
    Header = ['Layer','U','V','Constant','Constant_Error','MPV','MPV_Error','Sigma','Sigma_Error','Chi2','P','NDF']
    writer.writerow(Header)
    for l in range(max_layer):
        for u in range(max_uv):
            for v in range(max_uv):
                x = f'MAC2_cell_amplitude_l{l}_m0_u{u}_v{v}'
                h = d.Get(x)
                if not isinstance(h, ROOT.TH1F):
                    # not a histogram -> assuming not a valid cell
                    t.update()
                    continue
                # h is a valid histogram
                stats = [l,u,v]
                if h.GetEntries() < 10:
                    stats.extend(9*['NaN'])
                    writer.writerow(stats)
                    t.update()
                    continue
                # h is a valid histogram with at least 10 entries 
                fr = h.Fit(fl ,'BSQNM').Get()
                # stats = [l,u,v]
                for i in range(fr.NPar()):
                    stats.extend((fr.Parameter(i),fr.ParError(i)))
                stats.extend((fr.Chi2(),fr.Prob(),fr.Ndf()))
                writer.writerow(stats)
                #if l==20 and u==16 and v==17:
                    #fr.Print()
                    #rint(stats)
                    #input()
                t.update()

### i believe this is the code - working on something rn
