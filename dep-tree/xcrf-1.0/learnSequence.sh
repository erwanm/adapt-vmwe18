#$1:-v for verbose mode
#$2:delimiter pattern
#$3:path to the crf xml file
#$4:path to the corpus
#$5:path to the crf xml result file


java -cp lib/activation.jar:lib/colt.jar:lib/dom4j-1.6.1.jar:lib/jaxb-api.jar:lib/jaxb-impl.jar:lib/jaxen-1.1-beta-9.jar:lib/jsr173_1.0_api.jar:lib/mallet.jar:lib/log4j-1.3alpha-8.jar:lib/xcrf.jar xcrf.LearnSequence $*
