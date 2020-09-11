rem -- ebu_adm_renderer repository on graph-tool branch must be in PYTHONPATH

mv --force .\nesting1-1ao_with_1pf_reffing_1pf.wav .\nesting1-old.wav
ear-utils replace_axml -a .\nesting1-1ao_with_1pf_reffing_1pf.xml -g .\nesting1-old.wav .\nesting1-1ao_with_1pf_reffing_1pf.wav
python -m ear.cmdline.graph -d .\nesting1-1ao_with_1pf_reffing_1pf.wav nesting1

mv --force .\nesting1b-same_for_directspeakers.wav .\nesting1b-old.wav
ear-utils replace_axml -a .\nesting1b-same_for_directspeakers.xml -g .\nesting1b-old.wav .\nesting1b-same_for_directspeakers.wav
python -m ear.cmdline.graph -d .\nesting1b-same_for_directspeakers.wav nesting1b

mv --force .\nesting2-1ao_with_2pf_each_with_1cf.wav .\nesting2-old.wav
ear-utils replace_axml -a .\nesting2-1ao_with_2pf_each_with_1cf.xml -g .\nesting2-old.wav .\nesting2-1ao_with_2pf_each_with_1cf.wav
python -m ear.cmdline.graph -d .\nesting2-1ao_with_2pf_each_with_1cf.wav nesting2

mv --force .\nesting3-1ao_reffing_2ao_each_with_1pf_with_1cf.wav .\nesting3-old.wav
ear-utils replace_axml -a .\nesting3-1ao_reffing_2ao_each_with_1pf_with_1cf.xml -g .\nesting3-old.wav .\nesting3-1ao_reffing_2ao_each_with_1pf_with_1cf.wav
python -m ear.cmdline.graph -d .\nesting3-1ao_reffing_2ao_each_with_1pf_with_1cf.wav nesting3

mv --force .\nesting4-1ao_reffing_1ao_both_with_1pf_with_1cf.wav .\nesting4-old.wav
ear-utils replace_axml -a .\nesting4-1ao_reffing_1ao_both_with_1pf_with_1cf.xml -g .\nesting4-old.wav .\nesting4-1ao_reffing_1ao_both_with_1pf_with_1cf.wav
python -m ear.cmdline.graph -d .\nesting4-1ao_reffing_1ao_both_with_1pf_with_1cf.wav nesting4

mv --force .\nesting5-1ao_with_1pf_with_2cf.wav .\nesting5-old.wav
ear-utils replace_axml -a .\nesting5-1ao_with_1pf_with_2cf.xml -g .\nesting5-old.wav .\nesting5-1ao_with_1pf_with_2cf.wav
python -m ear.cmdline.graph -d .\nesting5-1ao_with_1pf_with_2cf.wav nesting5

mv --force .\nesting6-2ao_both_reffing_1ao_with_1pf_with_1cf.wav .\nesting6-old.wav
ear-utils replace_axml -a .\nesting6-2ao_both_reffing_1ao_with_1pf_with_1cf.xml -g .\nesting6-old.wav .\nesting6-2ao_both_reffing_1ao_with_1pf_with_1cf.wav
python -m ear.cmdline.graph -d .\nesting6-2ao_both_reffing_1ao_with_1pf_with_1cf.wav nesting6

mv --force .\nesting6b-2ao_parents_1ao_child_shared_and_1ao_child_with_one_parent.wav .\nesting6b-old.wav
ear-utils replace_axml -a .\nesting6b-2ao_parents_1ao_child_shared_and_1ao_child_with_one_parent.xml -g .\nesting6-old.wav .\nesting6b-2ao_parents_1ao_child_shared_and_1ao_child_with_one_parent.wav
python -m ear.cmdline.graph -d .\nesting6b-2ao_parents_1ao_child_shared_and_1ao_child_with_one_parent.wav nesting6b

mv --force .\nesting7-2ao_with_1pf_each_reffing_same_1cf.wav .\nesting7-old.wav
ear-utils replace_axml -a .\nesting7-2ao_with_1pf_each_reffing_same_1cf.xml -g .\nesting7-old.wav .\nesting7-2ao_with_1pf_each_reffing_same_1cf.wav
python -m ear.cmdline.graph -d .\nesting7-2ao_with_1pf_each_reffing_same_1cf.wav nesting7

mv --force .\nesting8-2ao_reffing_same_1pf_with_1cf.wav .\nesting8-old.wav
ear-utils replace_axml -a .\nesting8-2ao_reffing_same_1pf_with_1cf.xml -g .\nesting8-old.wav .\nesting8-2ao_reffing_same_1pf_with_1cf.wav
python -m ear.cmdline.graph -d .\nesting8-2ao_reffing_same_1pf_with_1cf.wav nesting8

pause
