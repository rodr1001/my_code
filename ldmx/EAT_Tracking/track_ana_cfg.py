from LDMX.Framework import ldmxcfg
p= ldmxcfg.Process("ana")
p.sequence = [ldmxcfg.Analyzer.from_file('TrackAnalyzer.cxx')]
import sys
p.inputFiles = sys.argv[1:]
p.histogramFile = f'hist_track_ana_{len(p.inputFiles)}.root'
