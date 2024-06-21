import ROOT
f = ROOT.TFile('hist-no-filtering.root')


d = f.Get('MAC2')

## creates all possible file names 
for l in range(34):
    for u in range (23):
        for v in range (23):
            x = 'MAC2_cell_amplitude_l'+str(l)+'_m0_u'+str(u)+'_v'+str(v)

### gets the histogram, named x

h = d.Get (x)
### checks that that h is a real valid histogram
if isinstance (h, ROOT.TH1F) == True:

### get the number of entries, only continues if number of entries is greater than 10
            E = h.GetEntries()
if E >= 10:

# fr = h.Fit("landau","S").Get()
# fr is a TFitResult
parameter_results = {
    fr.ParName(i): (fr.Parameter(i), fr.ParError(i))
    for i in range(fr.NPar())
}

f = ROOT.TFile('hist-no-filtering.root')
d = f.Get('MAC2')
with open ('cellstats.csv', 'w') as file:
    writer = csv.writer(file)
    Header = ['Layer','U','V','Constant','Constant_Error','MPV','MPV_Error','Sigma','Sigma_Error','Chi2','P','NDF']
    writer.writerow(Header)
    for l in range(1):
        for u in range (1):
            for v in range (24):
                x = 'MAC2_cell_amplitude_l'+str(l)+'_m0_u'+str(u)+'_v'+str(v)
                h = d.Get(x)
                if isinstance (h, ROOT.TH1F) == True:
                    E = h.GetEntries()
                    if E >= 10:
                        fr = h.Fit('landau','S').Get()
                        stats = [l,u,v]
                        for i in range(fr.NPar()):
                            stats.extend((fr.Parameter(i),fr.ParError(i)))
                        stats.extend((fr.Chi2(),fr.Prob(),fr.Ndf()))
                        writer.writerow(stats)

    file.close() 

### i believe this is the code - working on something rn
