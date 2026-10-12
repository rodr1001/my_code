

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--run-tag', default="TEST", help='extra name to include in output file')
#parser.add_argument('input_files', nargs='+', help='input root files')
parser.add_argument('input_dir', help='input directory of files to study')
parser.add_argument('-o','--output', default = 'hist.root',help='output file to write histograms into')
args = parser.parse_args()


from LDMX.Framework import ldmxcfg
p= ldmxcfg.Process("ana")
p.sequence = [ldmxcfg.Analyzer.from_file('ZoneZeroTrackAnalyzer.cxx')]
#p.inputFiles = args.input_files
p.inputDir(args.input_dir)
p.histogramFile = args.output


#p.inputFiles = args.input_files
#p.histogramFile = f'hist_track_ana_{len(p.inputFiles)}_{args.run_tag}.root'
#p.outputFiles = [f'skimzone_events__{len(p.inputFiles)}_{args.run_tag}.root']

