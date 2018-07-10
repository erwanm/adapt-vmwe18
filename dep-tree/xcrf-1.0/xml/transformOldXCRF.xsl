<?xml version="1.0" encoding="utf-8"?>
<xs:stylesheet version="2.0"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xs="http://www.w3.org/1999/XSL/Transform">

	<xs:output indent="yes" method="xml" encoding="utf-8" />

	<!-- setsoflabels modification -->
	<xs:template match="SetsOfLabels[count(.//*) = 0]">
		
	</xs:template>

	<!-- constraints -->

	<xs:template match="Constraint[@xsi:type = 'NodeConstraint']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="constraint"/>
			<xs:apply-templates select="ConstraintPredicate"/>
			<xs:apply-templates select="Yn" mode="CurrentNode-Constraint"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="CurrentNode-Constraint">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Constraint[@xsi:type = 'EdgeConstraint']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="constraint" />
			<xs:apply-templates select="ConstraintPredicate"/>
			<xs:apply-templates select="Yni" mode="Current-Parent-Constraint"/>
			<xs:apply-templates select="Yn" mode="Current-Parent-Constraint"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yni" mode="Current-Parent-Constraint">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="Current-Parent-Constraint">
		<xs:element name="Ypar">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Constraint[@xsi:type = 'TriangleConstraint']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="constraint" />
			<xs:apply-templates select="ConstraintPredicate"/>
			<xs:apply-templates select="Yni" mode="Current-Parent-Sibling-Constraint"/>
			<xs:apply-templates select="Yn" mode="Current-Parent-Sibling-Constraint"/>
			<xs:apply-templates select="Ynj" mode="Current-Parent-Sibling-Constraint"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yni" mode="Current-Parent-Sibling-Constraint">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="Current-Parent-Sibling-Constraint">
		<xs:element name="Ypar">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Ynj" mode="Current-Parent-Sibling-Constraint">
		<xs:element name="Ysib">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'NodeConstraint']" mode="constraint">		
		<xs:attribute name="xsi:type">CurrentNode-Constraint</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'EdgeConstraint']" mode="constraint">		
		<xs:attribute name="xsi:type">Current-Parent-Constraint</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'TriangleConstraint']" mode="constraint">		
		<xs:attribute name="xsi:type">Current-Parent-Sibling-Constraint</xs:attribute>
	</xs:template>

	<xs:template match="@*" mode="constraint">
		<xs:copy />
	</xs:template>
	
	<!-- tree features -->
	
	<xs:template match="Feature[@xsi:type = 'FeatureTree1']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="feature"/>
			<xs:apply-templates select="Yn" mode="CurrentNode"/>
			<xs:apply-templates select="TestX"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="CurrentNode">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Feature[@xsi:type = 'FeatureTree2']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="feature" />
			<xs:apply-templates select="Yni" mode="Current-Parent"/>
			<xs:apply-templates select="Yn" mode="Current-Parent"/>
			<xs:apply-templates select="TestX"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yni" mode="Current-Parent">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="Current-Parent">
		<xs:element name="Ypar">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Feature[@xsi:type = 'FeatureTree3']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="feature" />
			<xs:apply-templates select="Yni" mode="Current-Parent-Sibling"/>
			<xs:apply-templates select="Yn" mode="Current-Parent-Sibling"/>
			<xs:apply-templates select="Ynj" mode="Current-Parent-Sibling"/>
			<xs:apply-templates select="TestX"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yni" mode="Current-Parent-Sibling">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="Current-Parent-Sibling">
		<xs:element name="Ypar">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Ynj" mode="Current-Parent-Sibling">
		<xs:element name="Ysib">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Feature[@xsi:type = 'FeatureTree2Sibling']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="feature" />
			<xs:apply-templates select="Yni" mode="Current-Sibling"/>
			<xs:apply-templates select="Ynj" mode="Current-Sibling"/>
			<xs:apply-templates select="TestX"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yni" mode="Current-Sibling">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY"/>			
		</xs:element>
	</xs:template>

	<xs:template match="Ynj" mode="Current-Sibling">
		<xs:element name="Ysib">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<!-- sequence features -->

	<xs:template match="Feature[@xsi:type = 'FeatureSeq1']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="feature"/>
			<xs:apply-templates select="Yn" mode="CurrentToken"/>
			<xs:apply-templates select="TestX"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="CurrentToken">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Feature[@xsi:type = 'FeatureSeq2']">
		<xs:element name="{name(.)}">
			<xs:apply-templates select="@*" mode="feature" />
			<xs:apply-templates select="Yni" mode="Token-PrevToken"/>
			<xs:apply-templates select="Yn" mode="Token-PrevToken"/>
			<xs:apply-templates select="TestX"/>
		</xs:element>
	</xs:template>

	<xs:template match="Yni" mode="Token-PrevToken">
		<xs:element name="Ycur">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<xs:template match="Yn" mode="Token-PrevToken">
		<xs:element name="Yprev">
			<xs:apply-templates select="@*" mode="TestY" />			
		</xs:element>
	</xs:template>

	<!-- change names of feature type -->

	<xs:template match="@*[name() = 'xsi:type' and string() = 'FeatureTree1']" mode="feature">		
		<xs:attribute name="xsi:type">CurrentNode</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'FeatureTree2']" mode="feature">		
		<xs:attribute name="xsi:type">Current-Parent</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'FeatureTree3']" mode="feature">		
		<xs:attribute name="xsi:type">Current-Parent-Sibling</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'FeatureTree2Sibling']" mode="feature">		
		<xs:attribute name="xsi:type">Current-Sibling</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'FeatureSeq1']" mode="feature">		
		<xs:attribute name="xsi:type">CurrentToken</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'FeatureSeq2']" mode="feature">		
		<xs:attribute name="xsi:type">Token-PrevToken</xs:attribute>
	</xs:template>

	<xs:template match="@*" mode="feature">
		<xs:copy />
	</xs:template>

	<!-- change names of annotation type -->

	<xs:template match="@*[name() = 'xsi:type' and string() = 'AnnotationLabel']" mode="TestY">		
		<xs:attribute name="xsi:type">Label</xs:attribute>
	</xs:template>

	<xs:template match="@*[name() = 'xsi:type' and string() = 'AnnotationSeqLabel']" mode="TestY">		
		<xs:attribute name="xsi:type">SetOfLabels</xs:attribute>
	</xs:template>

	<xs:template match="@*" mode="TestY">
		<xs:copy />
	</xs:template>
	
	<!--  -->
	
	<xs:template match="@*">
		<xs:copy />
	</xs:template>
	
	<xs:template match="*">
		<xs:copy>
			<xs:apply-templates select="*|@*" />
		</xs:copy>
	</xs:template>


</xs:stylesheet>