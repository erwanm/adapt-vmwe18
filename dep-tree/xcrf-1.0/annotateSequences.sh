#$1:delimiter pattern
#$2:path to crf xml file
#$3:path to the file to be annotated
#$4:path to the result file

java -cp lib/activation.jar:lib/colt.jar:lib/dom4j-1.6.1.jar:lib/jaxb-api.jar:lib/jaxb-impl.jar:lib/jaxen-1.1-beta-9.jar:lib/jsr173_1.0_api.jar:lib/mallet.jar:lib/log4j-1.3alpha-8.jar:lib/xcrf.jar xcrf.AnnotateSequences $*
