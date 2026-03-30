 parallel -j2 --joblog report.log '
denv fire /local/cms/user/eichl008/ldmx/dimuon-calibration/config-physics-target-no-beam-smear.py {} > run-{}.log' ::: {2..10}

