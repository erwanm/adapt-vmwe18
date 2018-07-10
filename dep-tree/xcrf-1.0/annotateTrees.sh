# usage: java xcrf.XcrfOperations annotate [-dMargProb attributeNameForInternalNode attributeNameForTextLeaf attributeNameForAttribute | -cMargProb marginalProbabilitiesSetterPath] [-dAnnWriter attributeNameForInternalNode attributeNameForTextLeaf attributeNameForAttribute | -cAnnWriter annotationWriterPath] crfPath corpusPath resultPath
#
#    -cAnnWriter    use custom annotation writer
#    -cMargProb     use custom marginal probabilities setter
#    -dAnnWriter    use default annotation writer
#    -dMargProb     use default marginal probabilities setter

java -Xms1g -Xmx1g -jar lib/xcrf.jar annotate $*
