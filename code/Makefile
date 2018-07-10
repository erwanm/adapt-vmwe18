all: conll-tree-to-xml test-debug generate-xcrf-features-for-training get-class-from-xml cupt-to-bio-labels bio-to-cupt-labels merge-independent-labels cupt-list-exprs

get-class-from-xml: get-class-from-xml.cpp misc.o tsv.o xml_tree_explorer_get_class.o
	g++ -o get-class-from-xml -I/usr/include/libxml2/ -std=c++11  tsv.o misc.o  xml_tree_node_explorer.o xml_tree_explorer_get_class.o get-class-from-xml.cpp  -lxml2 -lm

generate-xcrf-features-for-training: generate-xcrf-features-for-training.cpp tsv.o xcrf_features.o deptree.o bidirectional_string_index.o xcrf_features_types.o misc.o xml_tree_node_explorer.o xcrf_xml_tree_check_xpath_tests.o  feature_selection.o fs_max_freq.o fs_binary_split.o fs_cond_entropy.o feature_selection_cliques.o fs_cond_entropy_clique.o
	g++ -o generate-xcrf-features-for-training -I/usr/include/libxml2/ -std=c++11  feature_selection.o fs_max_freq.o fs_binary_split.o fs_cond_entropy.o feature_selection_cliques.o fs_cond_entropy_clique.o xcrf_features.o deptree.o tsv.o bidirectional_string_index.o xcrf_features_types.o misc.o xml_tree_node_explorer.o xcrf_xml_tree_check_xpath_tests.o generate-xcrf-features-for-training.cpp  -lxml2 -lm

conll-tree-to-xml: conll-tree-to-xml.cpp tsv.o deptree.o misc.o
	g++ -o conll-tree-to-xml -I/usr/include/libxml2/ -std=c++11 tsv.o deptree.o misc.o conll-tree-to-xml.cpp -lxml2 -lm

test-debug: test-debug.cpp bidirectional_string_index.o bag_attr_val.h bag_attr_val.cpp
	g++ -o test-debug  -std=c++11 bidirectional_string_index.o test-debug.cpp

tsv.o: tsv.h tsv.cpp
	g++ -c -std=c++11 tsv.cpp

deptree.o: deptree.h deptree.cpp tsv.o misc.o
	g++ -c -std=c++11 -I/usr/include/libxml2/ deptree.cpp

bidirectional_string_index.o: bidirectional_string_index.h bidirectional_string_index.cpp
	g++ -c -std=c++11 bidirectional_string_index.cpp

# nope, due to c++ weird requirement with template classes
# see https://stackoverflow.com/questions/16270844/templated-class-returning-does-not-name-type
#bag_attr_val.o: bag_attr_val.h bag_attr_val.cpp bidirectional_string_index.o
#	g++ -c -std=c++11 bag_attr_val.cpp

xcrf_features.o: xcrf_features.h xcrf_features.cpp bidirectional_string_index.o xcrf_features_types.o xml_tree_node_explorer.o xcrf_xml_tree_check_xpath_tests.o feature_selection.o fs_max_freq.o fs_binary_split.o fs_cond_entropy.o
	g++ -c -std=c++11 -I/usr/include/libxml2/ xcrf_features.cpp

xcrf_features_types.o: xcrf_features_types.h xcrf_features_types.cpp
	g++ -c -std=c++11 xcrf_features_types.cpp

misc.o: misc.cpp misc.h
	g++ -c -std=c++11 misc.cpp

xml_tree_node_explorer.o: xml_tree_node_explorer.h xml_tree_node_explorer.cpp
	g++ -c -std=c++11 -I/usr/include/libxml2/ xml_tree_node_explorer.cpp

xcrf_xml_tree_check_xpath_tests.o: xcrf_xml_tree_check_xpath_tests.h xcrf_xml_tree_check_xpath_tests.cpp xml_tree_node_explorer.o
	g++ -c -std=c++11 -I/usr/include/libxml2/ xcrf_xml_tree_check_xpath_tests.cpp

xml_tree_explorer_get_class.o: xml_tree_explorer_get_class.h xml_tree_explorer_get_class.cpp xml_tree_node_explorer.o
	g++ -c -std=c++11 -I/usr/include/libxml2/ xml_tree_explorer_get_class.cpp

cupt-to-bio-labels: cupt-to-bio-labels.cpp misc.o tsv.o
	g++ -std=c++11 -o cupt-to-bio-labels misc.o tsv.o cupt-to-bio-labels.cpp

bio-to-cupt-labels: bio-to-cupt-labels.cpp misc.o tsv.o
	g++ -std=c++11 -o bio-to-cupt-labels misc.o tsv.o bio-to-cupt-labels.cpp

merge-independent-labels: merge-independent-labels.cpp  misc.o tsv.o
	g++ -std=c++11 -o merge-independent-labels  misc.o tsv.o merge-independent-labels.cpp

feature_selection.o: feature_selection.h feature_selection.cpp xcrf_features_types.o
	g++ -c -std=c++11 feature_selection.cpp

fs_max_freq.o: fs_max_freq.h fs_max_freq.cpp feature_selection.o
	g++ -c -std=c++11 fs_max_freq.cpp

fs_binary_split.o: fs_binary_split.h fs_binary_split.cpp feature_selection.o
	g++ -c -std=c++11 fs_binary_split.cpp

fs_cond_entropy.o: fs_cond_entropy.h fs_cond_entropy.cpp feature_selection.o
	g++ -c -std=c++11 fs_cond_entropy.cpp

feature_selection_cliques.o: feature_selection_cliques.h feature_selection_cliques.cpp feature_selection.o  xcrf_features_types.o
	g++ -c -std=c++11 feature_selection_cliques.cpp

fs_cond_entropy_clique.o: fs_cond_entropy_clique.h fs_cond_entropy_clique.cpp feature_selection_cliques.o
	g++ -c -std=c++11 fs_cond_entropy_clique.cpp

cupt-list-exprs: cupt-list-exprs.cpp misc.o tsv.o
	g++ -std=c++11 -o cupt-list-exprs misc.o tsv.o cupt-list-exprs.cpp


clean: 
	rm -f *.o conll-tree-to-xml test-debug generate-xcrf-features-for-training get-class-from-xml cupt-to-bio-labels bio-to-cupt-labels merge-independent-labels cupt-list-exprs

