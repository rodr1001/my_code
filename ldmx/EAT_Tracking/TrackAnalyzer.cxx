#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h" 
#include <iostream>
#include <optional>
#include <vector>

template<typename M>
M mag(const std::vector<M>& v) {
  M mag2{0};
  for (const auto& c: v) {
    mag2 += c*c;
  }
  return std::sqrt(mag2);
}
std::tuple<double,double> getImpactPoint(const ldmx::Track::TrackState &state) {
  return std::make_tuple(state.params[0], state.params[1]);
}

std::vector<const ldmx::Track*> sortByMomentum(const std::vector<ldmx::Track>& tracks) {
  std::vector<const ldmx::Track*> sorted_tracks;
  sorted_tracks.reserve(tracks.size());
  for (const auto& track : tracks) { sorted_tracks.emplace_back(&track); }
  std::sort(
    sorted_tracks.begin(),
    sorted_tracks.end(),
    [](const ldmx::Track* lhs, const ldmx::Track* rhs) {
      return mag(lhs->getMomentum()) > mag(rhs->getMomentum());
    }
  );
  return sorted_tracks;
}

class TrackAnalyzer : public framework::Analyzer {
 public:
  TrackAnalyzer(const std::string& name, framework::Process& p)
   : framework::Analyzer(name, p) {}
   ~TrackAnalyzer() = default;
   void onProcessStart() final;
   void analyze(const framework::Event& event) final;
};


void TrackAnalyzer::onProcessStart () {
  // define the histograms I want to fill
  getHistoDirectory(); 
  histograms_.create("event_tracks",
    "No. of Tracks per Event", 100, -0.5, 99.5);
  histograms_.create("n_hits",
    "No. of Hits for Lead Track", 100, -0.5, 13.5);
  histograms_.create("clean_event_tracks",
  "No. of Clean Tracks per Event", 100, -0.5, 10.5);
  //histograms_.create("hits_per_track",
    //"No. of Hits", 100, -0.5,15.5,
    //"No. of Tracks per Event", 100, -0.5, 99.5);
  histograms_.create("impact_point",
    "X [mm]", 200, -200.0, 200.0,
    "Y [mm]", 200, -200.0, 200.0);
  histograms_.create("leadtrk_impact_point",
    "X [mm]", 200, -200.0, 200.0,
    "Y [mm]", 200, -200.0, 200.0);

  histograms_.create("reco_momentum",
    "Reco Momentum (GeV)", 1000, 0, 10);
  histograms_.create("sim_momentum",
    "Sim Momentum (GeV)", 1000, 0, 10);
  histograms_.create("sim_momentum_impact_cuts",
    "Sim Momentum (GeV) with Impact Cuts ", 1000, 0, 10);
  histograms_.create("reco_leadtrk_momentum",
    "Leading Reco Momentum (GeV)",1000,0,10);
  histograms_.create("reco_leadtrk_momentum_impact_cuts",
    "Leading Reco Momentum (GeV) with Impact Point Cuts", 1000, 0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum",
  "Sim Momentum (GeV)", 100, 0, 10,
  "Reco Momentum (GeV)", 100,0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_impact_cuts",
  "Sim Momentum (GeV)",100,0,10,
  "Reco Momentum (GeV)", 100,0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_10_impact_cuts",
  "Sim Momentum (GeV)",100,0,10,
  "Reco Momentum (GeV)", 100,0, 10);

  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_10",
  "Sim Momentum (GeV)", 100, 0, 10,
  "Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_under_10",
  "Sim Momentum (GeV)", 100, 0, 10,
  "Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_chi2",
  "Chi2 of Lead Track",100, 0, 50);
  histograms_.create("leadtrk_chi2_nhits_10",
  "Chi2 of Lead Tracks with 10 Hits",100, 0, 50);
  histograms_.create("leadtrk_chi2_nhits_10_impact_cuts",
  "Chi2 of Lead Tracks with 10 Hits - impact point cuts",100, 0, 50);


}

void TrackAnalyzer::analyze(const framework::Event& event) {
  const auto& tracks{event.getCollection<ldmx::Track>("RecoilTracksClean", "")};
  // looks like: std::vector<ldmx::Track>
  histograms_.fill("clean_event_tracks", tracks.size());
  for (const auto& trk: tracks) {
    auto track_at_ecal{trk.getTrackState(ldmx::TrackStateType::AtECAL)};
    if (not track_at_ecal) {
      continue;
    }
    auto [x,y] = getImpactPoint(track_at_ecal.value());
    histograms_.fill("impact_point", x, y);
  }
  for (const auto& trk: tracks) {
    auto M_mag = mag(trk.getMomentum());
    histograms_.fill("reco_momentum", M_mag);
  }
  auto px = event.getObject<double>("PEFFPx","");
  auto py = event.getObject<double>("PEFFPy","");
  auto pz = event.getObject<double>("PEFFPz","");
  const std::vector<double> peffp{px,py,pz};
  auto peffp_mag = mag(peffp)/1000; //both reco and sim in GeV now
  histograms_.fill("sim_momentum", peffp_mag);
  //i want the leading track - the one with the highest momentum
  auto x = event.getObject<double>("PEFFx","");
  auto y = event.getObject<double>("PEFFy","");
  auto z = event.getObject<double>("PEFFz","");
  const std::vector<double> peffp{px,py,pz};
 if (x >=-11.5 & x<= 9 & y>= -38.5 & y<= 38.5)
 
 
 
 auto sorted_tracks{sortByMomentum(tracks)};
  
  if (sorted_tracks.size() != 0) {
    const auto& leadtrk{*(sorted_tracks[0])};
    //for (const auto& lead: leadtrk) {
    auto leadtrk_at_ecal{leadtrk.getTrackState(ldmx::TrackStateType::AtECAL)};
    if (leadtrk_at_ecal) {
      auto [x,y] = getImpactPoint(leadtrk_at_ecal.value());
      histograms_.fill("leadtrk_impact_point", x, y);
      auto leadtrk_momentum = mag(leadtrk.getMomentum());
      histograms_.fill("reco_leadtrk_momentum", leadtrk_momentum);
      histograms_.fill("leadtrk_reco_vs_sim_momentum",peffp_mag,leadtrk_momentum);
      histograms_.fill("leadtrk_chi2",leadtrk.getChi2());
      histograms_.fill("n_hits", leadtrk.getNhits());
       if (x >=-11.5 & x<= 9 & y>= -38.5 & y<= 38.5) {
         histograms_.fill("leadtrk_reco_vs_sim_momentum_impact_cuts", peffp_mag,leadtrk_momentum);
         if (leadtrk.getNhits() >= 10) {
           histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_10_impact_cuts",peffp_mag,leadtrk_momentum);
           histograms_.fill("leadtrk_chi2_nhits_10_impact_cuts",leadtrk.getChi2());
         }
       }
       if (leadtrk.getNhits() >= 10) {
         histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_10",peffp_mag,leadtrk_momentum);
         histograms_.fill("leadtrk_chi2_nhits_10",leadtrk.getChi2());
       } else {
           histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_under_10",peffp_mag, leadtrk_momentum);
        }
       }

    //if (peffp_mag < 4) {
      //histograms_.fill("leadtrk_reco_vs_sim_momentum_under_4",peffp_mag, leadtrk_momentum);}  
  }
  const auto& dtracks{event.getCollection<ldmx::Track>("RecoilTracks", "")};
  // looks like: std::vector<ldmx::Track>
  histograms_.fill("event_tracks", dtracks.size());
}


// i want to figure out the no. of tracks per event
//  RunHeader.description_.size()




DECLARE_ANALYZER(TrackAnalyzer);

