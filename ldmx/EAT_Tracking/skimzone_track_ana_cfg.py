from LDMX.Framework import ldmxcfg
p= ldmxcfg.Process("ana")
p.sequence = [ldmxcfg.Analyzer.from_file('SkimZoneTrackAnalyzer.cxx')]
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--run-tag', default="TEST", help='extra name to include in output file')
parser.add_argument('input_files', nargs='+', help='input root files')
args = parser.parse_args()
p.inputFiles = args.input_files
p.histogramFile = f'hist_track_ana_{len(p.inputFiles)}_{args.run_tag}.root'
#p.outputFiles = [f'skimzone_events__{len(p.inputFiles)}_{args.run_tag}.root']
#p.skimDefaultIsDrop()
#p.skimConsider('TrackAnalyzer')
