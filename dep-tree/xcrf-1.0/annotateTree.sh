# usage: java xcrf.annotateTree [-dMargProb attributeNameForInternalNode attributeNameForTextLeaf attributeNameForAttribute | -cMargProb marginalProbabilitiesSetterPath] [-dAnnWriter attributeNameForInternalNode attributeNameForTextLeaf attributeNameForAttribute | -cAnnWriter annotationWriterPath] crfPath filePath resultPath
#
#    -cAnnWriter    use custom annotation writer
#    -cMargProb     use custom marginal probabilities setter
#    -dAnnWriter    use default annotation writer
#    -dMargProb     use default marginal probabilities setter

java -Xms1g -Xmx1g -cp lib/activation.jar:lib/colt.jar:lib/dom4j-1.6.1.jar:lib/jaxb-api.jar:lib/jaxb-impl.jar:lib/jaxen-1.1-beta-9.jar:lib/jsr173_1.0_api.jar:lib/mallet.jar:lib/log4j-1.3alpha-8.jar:lib/saxon8.jar:lib/saxon8-xpath.jar:lib/saxon8-dom.jar:lib/commons-cli-1.0.jar:lib/xcrf.jar xcrf.AnnotateTree $*
