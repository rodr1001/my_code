from LDMX.Framework import ldmxcfg
p= ldmxcfg.Process("ana")
p.sequence = [ldmxcfg.processor_from_file('TrackAnalyzer.cxx', needs=['SimCore_Event', 'Tracking_Event'])]
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--run-tag', default="TEST", help='extra name to include in output file')
parser.add_argument('input_files', nargs='+', help='input root files')
args = parser.parse_args()
p.input_files = args.input_files
p.histogram_file = f'hist_track_ana_{len(p.input_files)}_{args.run_tag}.root'
p.output_files = [f'skimzone_events__{len(p.input_files)}_{args.run_tag}.root']
p.skim_default_is_drop()
p.skim_consider('TrackAnalyzer')
