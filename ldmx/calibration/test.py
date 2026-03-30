import ROOT
ROOT.gROOT.SetBatch(1)
f = ROOT.TFile('hist-no-filtering.root')
d = f.Get('MAC2')
c = ROOT.TCanvas()
x = 'MAC2_cell_amplitude_l20_m0_u16_v17'
h = d.Get(x)
fr = h.Fit('landau','SQ').Get()
print({
    fr.ParName(i): (fr.Parameter(i),fr.ParError(i))
    for i in range(fr.NPar())
})