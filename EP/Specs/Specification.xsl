<?xml version="1.0" ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">	
	<xsl:template match="/">
		<html>
			<head>
				<title><xsl:value-of select="/endpoint/@name" /></title>
				<style type="text/css">
				body { font-family:Helvetica, Verdana, sans-serif; font-size:10pt; }
				div.detail { border-bottom:solid 1px #A0A0A0; padding-bottom:10px; }
				h2 { margin:0px; background-color:#CC0099; color:white; padding:2px; margin-bottom:2px;display:block; }
				span.type { color:#A0A0A0; padding-right:5px; }
				code { background-color:#EFEFEF; border:solid 1px #A0A0A0; margin:2px; }
				</style>
			</head>
		</html>
		
		<body>
			<h1><xsl:value-of select="/endpoint/@friendly-name" /></h1>
			<p>
			This list specified the method/messages that devices need to implement to be compliant to this specification (<xsl:value-of select="/endpoint/@friendly-name" />). 
			A device complies if it implements all applicable messages following this specification.
			</p>
			<ul>
				<xsl:apply-templates select="//method" mode="toc" />
			</ul>
			
			<xsl:apply-templates select="//method" mode="detail" />
		</body>
	</xsl:template>
	
	<xsl:template match="method" mode="toc">
		<li>
			<a>
				<xsl:attribute name="href">#<xsl:value-of select="@id" /></xsl:attribute>
				<xsl:value-of select="@friendly-name" />
			</a>
		</li>
	</xsl:template>
	
	<xsl:template match="method" mode="detail">
		<div class="detail">
			<h2><a>
				<xsl:attribute name="name"><xsl:value-of select="@id" /></xsl:attribute>
				<xsl:value-of select="@friendly-name" />
			</a></h2>
			
			<b>Path(s): </b>
				<xsl:for-each select="path">
					<code><xsl:value-of select="." /></code>
				</xsl:for-each>
			
			<ul>
				<xsl:for-each select="parameter">
					<li>
						<span class="type"><xsl:value-of select="@type" /></span> <b><xsl:value-of select="@friendly-name" /></b>
						<xsl:if test="@min and @max">
							between <xsl:value-of select="@min" /> and <xsl:value-of select="@max" />
						</xsl:if>
						<xsl:if test="@default">
							, default <xsl:value-of select="@default" />
						</xsl:if>
					</li>
				</xsl:for-each>
			</ul>
			
			<p><xsl:value-of select="description" /></p>
		</div>
	</xsl:template>
</xsl:stylesheet>