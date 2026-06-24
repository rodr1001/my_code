#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h" 
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimParticle.h"
#include "DetDescr/SimSpecialID.h"
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
  int no_leading_electron_count = 0;
  int danger_count_ = 0;
  int TrackNumber = 0;
  int GoodTracks = 0;
  public:
  TrackAnalyzer(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~TrackAnalyzer() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
  void onProcessEnd() final {
    std::cout << "hits with no leading electron = " << no_leading_electron_count << std::endl;
    std::cout << "total danger count = " << danger_count_ << std::endl;
    std::cout << "total lead tracks at ECal = " << TrackNumber << std::endl;
    std::cout << "Total Lead Tracks, negatively charged, within beamspot and with 10 hits = " << GoodTracks<< std::endl;
  }
};


void TrackAnalyzer::onProcessStart () {
  // define the histograms I want to fill
  getHistoDirectory(); 
  // histograms_.create("event_tracks",
  //   "No. of Tracks per Event", 100, -0.5, 99.5);
  // histograms_.create("n_hits",
  //   "No. of Hits for Lead Track", 100, -0.5, 13.5);
  // histograms_.create("clean_event_tracks",
  //   "No. of Clean Tracks per Event", 100, -0.5, 10.5);
  //histograms_.create("hits_per_track",
  //"No. of Hits", 100, -0.5,15.5,
  //"No. of Tracks per Event", 100, -0.5, 99.5);
  histograms_.create("impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);
  histograms_.create("leadtrk_impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);
  histograms_.create("leadtrk_nhits_10_impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);
  histograms_.create("leadtrk_nhits_10_neg_charge_impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);

  //  histograms_.create("layerID", "Layer ID of Hits", 100, 0, 32);
  // histograms_.create("pdgID",   "PDG ID of Hits", 1000, 0, 50);

  histograms_.create("reco_momentum",
      "Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("sim_momentum",
      "Primary Electron Sim Momentum (GeV)", 100, 0, 10);

  //investigating charge of lead tracks
  // histograms_.create("leadtrk_charge", "Charge of Leading Track", 1000,-2,2);
  // histograms_.create("leadtrk_QoP", "QoP of Leading Track", 1000,-10,10);
  // histograms_.create("leadtrk_charge_DZ", "Charge of Leading Track in Danger Zone", 1000,-2,2); //DAnger Zone



  //histograms_.create("sim_momentum_impact_cuts",
  //  "Sim Momentum (GeV) with Impact Cuts ", 1000, 0, 10);
  histograms_.create("reco_leadtrk_momentum",
      "Lead Track Reco Momentum (GeV)",100,0,10);

  histograms_.create("reco_leadtrk_momentum_all_reqs",
      "Lead Track Reco Momentum (GeV) within Beamspot", 100, 0, 10);

  histograms_.create("leadtrk_reco_vs_sim_momentum",
      "Primary Electron Sim Momentum (GeV)", 100, 0, 10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);

  //  histograms_.create("leadtrk_reco_vs_sim_momentum_impact_cuts",
  //    "Primary Electron Sim Momentum (GeV)",100,0,10,
  //  "Lead Track Reco Momentum (GeV)", 100,0, 10);

  histograms_.create("leadtrk_reco_vs_sim_momentum_all_reqs",
      "Primary Electron Sim Momentum (GeV)",100,0,10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);

  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_10",
      "Primary Electron Sim Momentm (GeV)", 100, 0, 10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);


  histograms_.create("leadtrk_chi2",
      "Chi2 of Lead Track",100, 0, 20);
  histograms_.create("leadtrk_chi2_nhits_10",
      "Chi2 of Lead Tracks with 10 Hits",100, 0, 20);
  histograms_.create("leadtrk_chi2_nhits_10_impact_cuts",
      "Chi2 of Lead Tracks with 10 Hits within Impact Region",100, 0, 20);
  histograms_.create("leadtrk_all_req_reduced_chi2",
      "Chi2/NDF of Lead Track (ALL REQS)", 100, 0, 20);

  //creating histograms to investigate  the ECal scoring plane energy as the "true" "sim" momentum - replacing sim momentum in our graphs with the energy of the electron + photons nearby

  // histograms_.create("EScorePlane_momentum",
  //   "Momentum at Ecal Scoring Plane (MeV)", 1000, 0, 10);
  // histograms_.create("leading_electron_energy", "Leading Electron Energy (GeV)", 1000, 0, 10); // no radius involvment

  // R = 2 mm
  // histograms_.create("leading_E_vs_lead_E_w_energy_nearby_R2",
  // "Leading Electron Energy (GeV)", 1000, 0, 10,
  //  "Leading Electron Energy Estimate (GeV)", 1000, 0, 10);
  //need a better phrase for Lead E and Nearby E
  //  histograms_.create("leadtrk_reco_vs_Lead_E_w_nearby_R2", 
  //    "Energy Estimate", 100, 0, 10,
  //  "Reco Momentum (GeV)", 100, 0, 10);
  //applying the hits = 10 cut
  // histograms_.create("leadtrk_reco_nhits_10_vs_Lead_E_w_nearby_R2",
  //   "Energy Estimate (GeV)", 100, 0, 10,
  // "Reco Momentum (GeV)", 100, 0, 10);


  //histograms_.create("leadtrk_reco_nhits_10_impact_cuts_vs_Energy_Estimate_R2",
  //    "Energy Estimate (GeV)", 100,0,10,
  //  "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  // histograms_.create("difference_R2",
  //   "Energy Difference (Reco - Estimate (GeV)", 100,-10,10);
  //  histograms_.create("energy_diff_vs_chi2_R2",
  //    "Chi2 of Lead Track", 100, 0, 20,
  //  "Energy Difference (Reco - Estimate)(GeV)", 100,-10,10);


  //R=5 - creating them now, not filling them, probably should write a piece of code that goes through this list of R
  // histograms_.create("leadtrk_reco_nhits_10_impact_cuts_vs_Energy_Estimate_R5",
  //   "Energy Estimate (GeV)", 100,0,10,
  // "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  //  histograms_.create("difference_R5",
  //    "Energy Difference (Reco - Estimate (GeV)", 100,-10,10);
  // histograms_.create("energy_diff_vs_chi2_R5",
  //   "Chi2 of Lead Track", 100, 0, 20,
  // "Energy Difference (Reco - Estimate)(GeV)", 100,-10,10);


  // R= 10 

  //  histograms_.create("leadtrk_reco_nhits_10_impact_cuts_vs_Energy_Estimate_R10",
  //    "Energy Estimate within 10 mm (GeV)", 100,0,10,
  //  "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  //  histograms_.create("difference_R10",
  //    "Energy Difference (Reco - Estimate (GeV)", 100,-10,10);
  //  histograms_.create("energy_diff_vs_chi2_R10",
  //    "Chi2 of Lead Track", 100, 0, 20,
  //  "Energy Difference (Reco - Estimate)(GeV)", 100,-10,10);




  histograms_.create("leadtrk_reco_all_reqs_vs_Energy_Estimate_Beamspot",
      "Energy Estimate within Beamspot (GeV)", 100,0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("difference_Beamspot",
      "Energy Difference (Reco - Estimate (GeV)", 100,-10,10);
  histograms_.create("energy_diff_vs_chi2_Beamspot",
      "Chi2 of Lead Track", 100, 0, 20,
      "Energy Difference (Reco - Estimate)(GeV)", 100,-10,10);



}

void TrackAnalyzer::analyze(const framework::Event& event) {

  //looking at EcalScroingPlaneHits - we are under the hood of PEFF
  const auto& hits{event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits", "")}; // this is closed - can stay here

  // radii list?? tech a vector
  //  std::vector<int> radii = {2}; // {2,5,10};

  auto thresh = 6;

  //creating a list of sorted hits
  std::vector<const ldmx::SimTrackerHit*> sorted_hits;
  sorted_hits.reserve(hits.size());
  for (const auto& hit: hits){
    ldmx::SimSpecialID hit_id = (hit.getID());
    // skip hits that are not particles _enterting_ (positive z-momentum)
    // the _front_ (plane == 31)
    if (hit_id.plane() != 31 or hit.getMomentum().at(2) < 0) {
      continue;
    }
    sorted_hits.emplace_back(&hit);
  } //list created

  std::sort(
      sorted_hits.begin(),
      sorted_hits.end(),
      [](const ldmx::SimTrackerHit* lhs, const ldmx::SimTrackerHit* rhs) {
      return mag(lhs->getMomentum()) > mag(rhs->getMomentum());
      }
      ); // list sorted

  //still within the analyzer but nothing else

  // 1: find highest momentum/energy electron
  const ldmx::SimTrackerHit* leading_electron{nullptr};
  for (const auto* hit: sorted_hits) {
    if (hit->getPdgID() == 11) {
      leading_electron = hit;
      break;
    }
  }
  if (leading_electron == nullptr) {
    no_leading_electron_count ++;
    //std::cout << "Never found an electron entering the ECal in event " << event.getEventNumber() << std::endl; 
    //progress bar of sorts - need to find a way to tidy this
    return;
  }
  auto leading_electron_energy = (leading_electron->getEnergy())/1000;
  //histograms_.fill("leading_electron_energy", leading_electron_energy);

  // Now creating the Tracks collection - this is where I have Reco and Sim Momentum - i should be out of everything except the analyse call
  // these are not radius dependent, don't want to run this every time

  const auto& tracks{event.getCollection<ldmx::Track>("RecoilTracksClean", "")};
  // looks like: std::vector<ldmx::Track>
  // histograms_.fill("clean_event_tracks", tracks.size());

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
    // this works auto pID = trk.getPdgID() ;
    // std::cout << pID;
  }
  auto px = event.getObject<double>("PEFFPx","");
  auto py = event.getObject<double>("PEFFPy","");
  auto pz = event.getObject<double>("PEFFPz","");
  const std::vector<double> peffp{px,py,pz};
  auto peffp_mag = mag(peffp)/1000; //both reco and sim in GeV now
  histograms_.fill("sim_momentum", peffp_mag);
  //i want the leading track - the one with the highest momentum
  //
  auto sorted_tracks{sortByMomentum(tracks)};

  //only looking at tracks that actually have stuff in them 
  if (sorted_tracks.size() != 0) {
    const auto& leadtrk{*(sorted_tracks[0])};

    //LEAD TRACK STUFF

    //picked the largest momentum track
    auto leadtrk_at_ecal{leadtrk.getTrackState(ldmx::TrackStateType::AtECAL)};
    //only looking a thtose that actually show up at the ECAL

    if (leadtrk_at_ecal) {
      TrackNumber++;
      //std::cout << "Lead Track No. " << TrackNumber << std::endl;

      auto [x,y] = getImpactPoint(leadtrk_at_ecal.value());
      histograms_.fill("leadtrk_impact_point", x, y);

      auto leadtrk_momentum = mag(leadtrk.getMomentum());
      histograms_.fill("reco_leadtrk_momentum", leadtrk_momentum);

      histograms_.fill("leadtrk_reco_vs_sim_momentum",peffp_mag,leadtrk_momentum);

      auto QoP = leadtrk.getQoP();
      auto charge = QoP*leadtrk_momentum;

      auto nhits = leadtrk.getNhits();
      auto chi2  = leadtrk.getChi2();

      histograms_.fill("leadtrk_chi2", chi2);

      bool inBeamspot = (x >= -11.5 && x <= 9) && (y >= -38.5 && y <= 38.5);
      bool hasEnoughHits  = (nhits >= 10);

      // before radius loop because that's irrelevant
      if (hasEnoughHits) {
        histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_10", peffp_mag, leadtrk_momentum);
        histograms_.fill("leadtrk_nhits_10_impact_point", x, y);
        if (charge < 0) {
          histograms_.fill("leadtrk_nhits_10_neg_charge_impact_point", x,y);
          if  (inBeamspot) {
            histograms_.fill("leadtrk_reco_vs_sim_momentum_all_reqs", peffp_mag, leadtrk_momentum);
            histograms_.fill("reco_leadtrk_momentum_all_reqs", leadtrk_momentum);
            histograms_.fill("leadtrk_all_req_reduced_chi2", chi2/4);

            GoodTracks ++;
            // This is currently nested within RECO, clean tracks, negatively charged, in impact region and with enough hits
            // ECAL Scoring plane stuff

            //for (int r: radii) 
            //// 2: loop through hits again and collect photons that are within radius of this electron
            double nearby_energy{0.0};
            std::set<int> used_ids;
            for (const auto* hit: sorted_hits) {
              // skip our leading eleleadtrk_nhits_10_impact_pointctron
              if (hit == leading_electron) {
                continue;
              }
              const auto& leading_electron_pos = leading_electron->getPosition();

              // could filter for things here
      
     if ((hit->getPdgID() == 11) or (hit->getPdgID() == -11) or (hit->getPdgID() == 22)) {
                const auto& pos = hit->getPosition();
                
                bool inImpactRegion = (pos[0] >= -20 && pos[0] <= 20) && ( pos[1] >= -50 && pos[1] <= 50);
                //auto dx = leading_electron_pos[0] - pos[0];
                //auto dy = leading_electron_pos[1] - pos[1];
                // if (dx*dx + dy*dy < r*r) {
                if (inImpactRegion and used_ids.find(hit->getTrackID())==used_ids.end()) {
                  used_ids.insert(hit->getTrackID());
                  nearby_energy += (hit->getEnergy()/1000);
                }
              }
            } //slose sorted hits & adding nearby energy
            auto leading_electron_energy = (leading_electron->getEnergy())/1000;
            auto energy_estimate = leading_electron_energy + nearby_energy ;
            auto difference = leadtrk_momentum - energy_estimate;

            histograms_.fill("leadtrk_reco_all_reqs_vs_Energy_Estimate_Beamspot", energy_estimate, leadtrk_momentum);
            histograms_.fill("difference_Beamspot", difference);
            histograms_.fill("energy_diff_vs_chi2_Beamspot", chi2, difference);

            //histograms_.fill(("leadtrk_reco_nhits_10_impact_cuts_vs_Energy_Estimate_R"+std::to_string(r)).c_str(), energy_estimate, leadtrk_momentum);
             //histograms_.fill(("difference_R" +std::to_string(r)).c_str(), difference);
            //histograms_.fill(("energy_diff_vs_chi2_R"+std::to_string(r)).c_str() , chi2, difference);
            //if (energy_estimate > 8.00) {
            if ((energy_estimate < 4.00) && (leadtrk_momentum > thresh)) {
              //std::cout << "For Radius " << r << " mm :" << std::endl;
              danger_count_++;
              auto PDG_id = leadtrk.getPdgID();
              std::cout << "\nDanger Event Found No. " << danger_count_ << std::endl;
              std::cout << "Reco momentum = " << leadtrk_momentum << "GeV" << std::endl;
              std::cout << "energy estimate = " << energy_estimate <<std::endl;
              const auto& leading_electron_pos = leading_electron->getPosition();
              //auto [x,y] = getImpactPoint(leadtrk_at_ecal.value());
              //std::cout << "Lead Track position = (" << x << "," << y << ")" << std::endl;
              std::cout << "Lead electron position at ECal = (" << leading_electron_pos[0] << "," << leading_electron_pos[1] << "," <<leading_electron_pos[2] << ")" << std::endl; 
              auto HasNonElectron =100; //edited from zero for purposes of examing 8+ GeV energy estimate
              for (const auto& [track_id, particle]: event.getMap<int, ldmx::SimParticle>("SimParticles", "")) {
                auto particle_pdg = particle.getPdgID() ;
               //if (particle_pdg != 11) { //what if i only printed out those that aren't electrons ... if there were high energy electrons we'd have selected it instead
                  //HasNonElectron = 1;
                  std::cout << track_id << " -> PDG = "
                  << particle_pdg << " E = " << particle.getEnergy()/1000 << " GeV" << "\n";
                 // << " Gen Status = "
                 // << particle.getGenStatus() << "\n";
                  // << particle.getMomentum()[1]/1000 << ", "
                  // << particle.getMomentum()[2]/1000 << " ) GeV"
                  //<< " Generated at = ("
                 // << particle.getVertex()[0] << ", "
                 // << particle.getVertex()[1] << ", "
                 // << particle.getVertex()[2] << " ) mm"
                 // << "\n";
                  for (const auto& hit: hits) {
                    auto ESPH_track_id = hit.getTrackID();
                    //std::cout << "Ecal Scoring Plane Hit Track ID" << ESPH_track_id << std::endl;
                    if (ESPH_track_id == track_id) {
                      auto pos = hit.getPosition();
                      auto dx = leading_electron_pos[0] - pos[0];
                      auto dy = leading_electron_pos[1] - pos[1];
                      auto distance = sqrt((dx*dx)+(dy*dy));
                      std::cout << "hit ECal at " << pos[0] << ", " << pos[1] << " a distance " << distance << " mm from leading electron with E = " << hit.getEnergy()/1000 << std::endl;
                    }// else { 
                      //  std::cout << "Did not hit ECal" << std::endl;
                    }   
                  } //ends readout of where the simparticle hits Ecal
               // } //else says mysimparticle is an electron
             // } //already went throguh all the simparticles to see if there was a non electron
              if (HasNonElectron = 0) {
                //std::cout << "Value should be 0. Is it? " << HasNonElectron << std::endl;
                std::cout << "No non-electrons found for this danger event" << std::endl;
                for (const auto& [track_id, particle]: event.getMap<int, ldmx::SimParticle>("SimParticles", "")) {
                  auto particle_pdg = particle.getPdgID() ;
                  std::cout << track_id << " -> PDG = "
                  << particle_pdg << " E = " << particle.getEnergy()/1000 << " GeV"
                  << " p = ("
                  << particle.getMomentum()[0]/1000 << ", "
                  << particle.getMomentum()[1]/1000 << ", "
                  << particle.getMomentum()[2]/1000 << " ) GeV"
                  << " Generated at = ("
                  << particle.getVertex()[0] << ", "
                  << particle.getVertex()[1] << ", "
                  << particle.getVertex()[2] << " ) mm"
                  << "\n";
                  for (const auto& hit: hits) {
                    auto ESPH_track_id = hit.getTrackID();
                    //std::cout << "Ecal Scoring Plane Hit Track ID" << ESPH_track_id << std::endl;
                    if (ESPH_track_id == track_id) {
                      auto pos = hit.getPosition();
                      auto dx = leading_electron_pos[0] - pos[0];
                      auto dy = leading_electron_pos[1] - pos[1];
                      auto distance = sqrt((dx*dx)+(dy*dy));
                      std::cout << "hit ECal at " << pos[0] << ", " << pos[1] << " a distance " << distance << " mm from leading electron" <<std::endl;
                    } else {
                      std::cout << "Did not hit ECal" << std::endl;
                    }
                 } 
                } //no non electron readout // did i or did i not have a non electron in my simparticles
              } //simparticle further info
             }//DANGER ZONE READ OUT
           // } //exit radii loop
          } // exit impactregion req
        } //exit charge req
      } // exit hit req
    }//exit lead track at ECal loop
  }// close sorted tracks 

  // const auto& dtracks{event.getCollection<ldmx::Track>("RecoilTracks", "")};
  // looks like: std::vector<ldmx::Track>
  // histograms_.fill("event_tracks", dtracks.size());


  // i want to figure out the no. of tracks per event
  //  RunHeader.description_.size()

} //close analyser


DECLARE_ANALYZER(TrackAnalyzer);
