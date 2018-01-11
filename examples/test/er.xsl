<?xml version='1.0' ?>
<xsl:stylesheet  xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
<html>
<head>
<title>MPICH Error Report</title>
<meta http-equiv="refresh" content="240"></meta>
</head>
<body>
    <h1> Error Report </h1>
    <table border="2" bgcolor="Green">
    <xsl:apply-templates select="MPITESTRESULTS"/>
    </table>
</body>
</html>
</xsl:template>

<xsl:template match="MPITESTRESULTS">
    <xsl:apply-templates select="MPITEST"/>
</xsl:template>

<xsl:template match="MPITEST">
    <tr bgcolor="white">
    <td valign="top">	
    <xsl:value-of select="NAME"/>
    </td><td valign="top">
    <xsl:value-of select="STATUS"/>
    </td><td WIDTH="40%"><pre>
    <xsl:value-of select="TESTDIFF"/>
    </pre>
    </td>
    </tr>
</xsl:template>
</xsl:stylesheet>
