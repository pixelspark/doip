<?xml version='1.0' ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">	
  <xsl:template match="/">
    <html>
      <head>
        <title><xsl:value-of select="/api/@name" /></title>
        
        <style type="text/css">
          BODY,TD,TR,TH {
            font-family:Verdana,Arial,sans-serif;
            font-size:11px;
          }
          
          TD {
            background-color:White;
          }
          
          H1 {
            font-size:15px;
            margin:0px;
          }
          
          H2 {
            font-size:15px;
            margin:0px;
          }
          
          HR {
            height:0px;
            border-top:solid 1px #CCCCCC;
            margin:0px;
            padding:0px;
          }
          
          DIV.class {
            width:95%;
            padding:5px;
            background-color:#EFEFEF;
            margin-bottom:5px;
            -moz-border-radius:5px;
          }
          
          DIV.internal {
            color:#A0A0A0;
          }
          
          TABLE.members {
            width:100%;
          }
          
          A.type {
            font-style:italic;
            color:#0099CC;
          }
          
          TD.member-name {
            font-weight:bold;
            text-align:right;
            width:130px;
          }
          
          DIV.description {
            margin-bottom:5px;
            margin-top:5px;
          }
          
          SPAN.not-required {
            color:Gray;
          }
          
          TD.constructor {
            color:Green;
          }
          
          SPAN.implement {
            color:#009900;
            font-weight:bold;
          }
          
          SPAN.operator {
            color:#000099;
            font-weight:bold;
          }
          
          TD.return-type {
            text-align:right;
            width:130px;
          }
          
          TD.param {
            /*word-wrapping:none;*/
          }
          
          TD.desc {
            overflow:auto;
          }
          
          H2 A:target {
            color:Red;
          }
        </style>
      </head>
      
      <body>
        <h1><xsl:value-of select="/api/@name" /></h1>
        <br/>
        <xsl:apply-templates select="/api/class[not(@internal='yes')]">
          <xsl:sort select="@name" />
        </xsl:apply-templates>
        
        <div class="internal-classes">
          <xsl:apply-templates select="/api/class[@internal='yes']">
            <xsl:sort select="@name" />
          </xsl:apply-templates>
        </div>
      </body>
    </html>
  </xsl:template>
  
  <xsl:template match="class">
    <div>
      <xsl:attribute name="class">class <xsl:if test="@internal = 'yes' ">internal</xsl:if></xsl:attribute>
      
      <h2><a>
        <xsl:attribute name="name">type-<xsl:value-of select="@name" /></xsl:attribute>
        <xsl:value-of select="@name" />
      </a></h2>
      
      <div class="description"><xsl:value-of select="text()" /></div>
      
      <xsl:apply-templates select="implements" />
      
      <table class="members">
        <xsl:apply-templates select="constructor" />
        <xsl:apply-templates select="member">
          <xsl:sort select="@name" />
        </xsl:apply-templates>
      </table>
    </div>
  </xsl:template>
  
  <xsl:template match="member">
    <tr>
      <td class="return-type">
        <a class="type">
          <xsl:attribute name="href">#type-<xsl:value-of select="@type" /></xsl:attribute>
          <xsl:value-of select="@type"/>
        </a>
      </td>
      <td class="member-name"><xsl:value-of select="@name" /></td>
      <td class="param">
        <xsl:if test="count(param) &gt; 0">
          <xsl:text>(</xsl:text>
          <xsl:apply-templates select="param" /><xsl:text>)</xsl:text>
        </xsl:if>
      </td>
      
      <td class="desc">
        <xsl:value-of select="text()" />
        <xsl:if test="@static">
          <span class="operator"> static</span>
        </xsl:if>
        <xsl:if test="@operator">
          <span class="operator">equivalent to operator <xsl:value-of select="@operator" /></span>
        </xsl:if>
      </td>
    </tr>
  </xsl:template>
  
  <xsl:template match="implements">
    <span class="implement">implements 
      <a class="type">
        <xsl:attribute name="href">#type-<xsl:value-of select="text()"/></xsl:attribute>
        <xsl:value-of select="text()"/>
      </a>
    </span>
  </xsl:template>
  
  <xsl:template match="constructor">
    <tr>
      <td>
      </td>
      <td class="member-name">new <xsl:value-of select="../@name" /></td>
      <td class="param">
          <xsl:text>(</xsl:text>
          <xsl:apply-templates select="param" />
          <xsl:text>)</xsl:text>
      </td>
      
      <td><xsl:value-of select="text()" /></td>
    </tr>
  </xsl:template>
  
  <xsl:template match="param">
    <span>
      <xsl:attribute name="class">param <xsl:if test="@required='no'">not-required</xsl:if></xsl:attribute>
    
       <abbr>
        <xsl:attribute name="title"><xsl:value-of select="text()" /></xsl:attribute>
        <xsl:value-of select="@name"/>
      </abbr>
      <xsl:text>=</xsl:text>
      <a class="type">
        <xsl:attribute name="href">#type-<xsl:value-of select="@type" /></xsl:attribute>
        <xsl:value-of select="@type"/>
      </a>
    </span>
    
    <xsl:if test="not(position()= last())">
      <xsl:text>, </xsl:text>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
