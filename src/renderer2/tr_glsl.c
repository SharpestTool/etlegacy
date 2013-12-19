/*
===========================================================================
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_glsl.c

#include "tr_local.h"

static char *complieMacroNames[] = 
{
	"USE_ALPHA_TESTING",
	"USE_PORTAL_CLIPPING",
    "USE_FRUSTUM_CLIPPING",
    "USE_VERTEX_SKINNING",
    "USE_VERTEX_ANIMATION",
    "USE_DEFORM_VERTEXES",
    "USE_TCGEN_ENVIRONMENT",
    "USE_TCGEN_LIGHTMAP",
    "USE_NORMAL_MAPPING",
    "USE_PARALLAX_MAPPING",
    "USE_REFLECTIVE_SPECULAR",
    "USE_SHADOWING",
	"TWOSIDED",
    "EYE_OUTSIDE",
    "BRIGHTPASS_FILTER",
    "LIGHT_DIRECTIONAL",
    "USE_GBUFFER"
};

// These must be in the same order as in uniform_t in tr_local.h.
static uniformInfo_t uniformsInfo[] =
{
	{ "u_DiffuseMap",                GLSL_INT    },
	{ "u_LightMap",                  GLSL_INT    },
	{ "u_NormalMap",                 GLSL_INT    },
	{ "u_DeluxeMap",                 GLSL_INT    },
	{ "u_SpecularMap",               GLSL_INT    },

	{ "u_TextureMap",                GLSL_INT    },
	{ "u_LevelsMap",                 GLSL_INT    },

	{ "u_ScreenImageMap",            GLSL_INT    },
	{ "u_ScreenDepthMap",            GLSL_INT    },

	{ "u_ShadowMap",                 GLSL_INT    },
	{ "u_ShadowMap2",                GLSL_INT    },
	{ "u_ShadowMap3",                GLSL_INT    },

	{ "u_ShadowMvp",                 GLSL_MAT16  },
	{ "u_ShadowMvp2",                GLSL_MAT16  },
	{ "u_ShadowMvp3",                GLSL_MAT16  },

	{ "u_DiffuseTexMatrix",          GLSL_VEC4   },
	{ "u_DiffuseTexOffTurb",         GLSL_VEC4   },
	{ "u_Texture1Env",               GLSL_INT    },

	{ "u_TCGen0",                    GLSL_INT    },
	{ "u_TCGen0Vector0",             GLSL_VEC3   },
	{ "u_TCGen0Vector1",             GLSL_VEC3   },

	{ "u_DeformGen",                 GLSL_INT    },

	{ "u_ColorGen",                  GLSL_INT    },
	{ "u_AlphaGen",                  GLSL_INT    },
	{ "u_Color",                     GLSL_VEC4   },
	{ "u_BaseColor",                 GLSL_VEC4   },
	{ "u_VertColor",                 GLSL_VEC4   },

	{ "u_DlightInfo",                GLSL_VEC4   },
	{ "u_LightForward",              GLSL_VEC3   },
	{ "u_LightUp",                   GLSL_VEC3   },
	{ "u_LightRight",                GLSL_VEC3   },
	{ "u_LightOrigin",               GLSL_VEC4   },
	{ "u_LightRadius",               GLSL_FLOAT  },
	{ "u_AmbientLight",              GLSL_VEC3   },
	{ "u_DirectedLight",             GLSL_VEC3   },

	{ "u_PortalRange",               GLSL_FLOAT  },

	{ "u_FogDistance",               GLSL_VEC4   },
	{ "u_FogDepth",                  GLSL_VEC4   },
	{ "u_FogEyeT",                   GLSL_FLOAT  },
	{ "u_FogColorMask",              GLSL_VEC4   },

	{ "u_ModelMatrix",               GLSL_MAT16  },
	{ "u_ModelViewProjectionMatrix", GLSL_MAT16  },

	{ "u_Time",                      GLSL_FLOAT  },
	{ "u_VertexLerp",                GLSL_FLOAT  },
	{ "u_MaterialInfo",              GLSL_VEC2   },

	{ "u_ViewInfo",                  GLSL_VEC4   },
	{ "u_ViewOrigin",                GLSL_VEC3   },
	{ "u_ViewForward",               GLSL_VEC3   },
	{ "u_ViewLeft",                  GLSL_VEC3   },
	{ "u_ViewUp",                    GLSL_VEC3   },

	{ "u_InvTexRes",                 GLSL_VEC2   },
	{ "u_AutoExposureMinMax",        GLSL_VEC2   },
	{ "u_ToneMinAvgMaxLinear",       GLSL_VEC3   },

	{ "u_PrimaryLightOrigin",        GLSL_VEC4   },
	{ "u_PrimaryLightColor",         GLSL_VEC3   },
	{ "u_PrimaryLightAmbient",       GLSL_VEC3   },
	{ "u_PrimaryLightRadius",        GLSL_FLOAT  },

	//from XREAL
	{ "u_ColorTextureMatrix",		 GLSL_MAT16  },
	{ "u_DiffuseTextureMatrix",		 GLSL_MAT16  },
	{ "u_NormalTextureMatrix",		 GLSL_MAT16  },
	{ "u_SpecularTextureMatrix",	 GLSL_MAT16  },
	{ "u_AlphaTest",				 GLSL_INT    },
	{ "u_ColorModulate",			 GLSL_VEC4   },
	{ "u_BoneMatrix",				 GLSL_MAT16  },
	{ "u_VertexInterpolation",		 GLSL_FLOAT  },
	{ "u_PortalPlane",				 GLSL_VEC4   },
	{ "u_CurrentMap",				 GLSL_INT    },
	{ "u_ColorMap",					 GLSL_INT    },
	{ "u_AmbientColor",				 GLSL_VEC3   },
	{ "u_LightDir",					 GLSL_VEC3   },
	{ "u_LightColor",				 GLSL_VEC3   },
	{ "u_LightScale",				 GLSL_FLOAT  },
	{ "u_LightWrapAround",			 GLSL_FLOAT  },
	{ "u_LightAttenuationMatrix",    GLSL_MAT16  }, 
	{ "u_LightFrustum",				GLSL_VEC4ARR}, // VEC4 [6]
	{ "u_ShadowTexelSize",			 GLSL_FLOAT  },
	{ "u_ShadowBlur",				 GLSL_FLOAT  },
	{ "u_ShadowMatrix",			   GLSL_MAT16ARR}, //MAT16 [5]
	{ "u_ShadowParallelSplitDistances", GLSL_VEC4},
	{ "u_ViewMatrix",				 GLSL_MAT16  },
	{ "u_ModelViewMatrix",			 GLSL_MAT16  },
	{ "u_ModelViewMatrixTranspose",  GLSL_MAT16  },
	{ "u_ProjectionMatrixTranspose", GLSL_MAT16  },
	{ "u_UnprojectMatrix",			 GLSL_MAT16  },
	{ "u_DepthScale",				 GLSL_FLOAT  },
	{ "u_EnvironmentInterpolation",  GLSL_FLOAT  },
	{ "u_DeformParms",				 GLSL_FLOATARR}, // FLOAT [MAX_SHADER_DEFORM_PARMS]
	{ "u_FogDistanceVector",		 GLSL_VEC4   },
	{ "u_FogDepthVector",			 GLSL_VEC4   },
	{ "u_DeformMagnitude",			 GLSL_FLOAT  },
	{ "u_HDRKey",					 GLSL_FLOAT  },
	{ "u_HDRAverageLuminance",		 GLSL_FLOAT  },
	{ "u_HDRMaxLuminance",			 GLSL_FLOAT  },
	{ "u_RefractionIndex",			 GLSL_FLOAT  },
	{ "u_FogDensity",				 GLSL_FLOAT  },
	{ "u_FogColor",					 GLSL_VEC3   },
	{ "u_FresnelPower",				 GLSL_FLOAT  },
	{ "u_FresnelScale",				 GLSL_FLOAT  },
	{ "u_FresnelBias",				 GLSL_FLOAT  },
	{ "u_BlurMagnitude",			 GLSL_FLOAT  },
	{ "u_NormalScale",				 GLSL_FLOAT  },
	{ "u_ShadowCompare",			 GLSL_FLOAT  },
	{ "u_EtaRatio",					 GLSL_VEC3   }
};

#define FILE_HASH_SIZE      4096
#define MAX_SHADER_DEF_FILES 1024
#define DEFAULT_SHADER_DEF NULL
static programInfo_t *hashTable[FILE_HASH_SIZE];

static char *definitionText;

/*
================
return a hash value for the filename
This function is cloned to many files, should be moved to common->
================
*/
static long generateHashValue(const char *fname)
{
	int  i    = 0;
	long hash = 0;
	char letter;

	while (fname[i] != '\0')
	{
		letter = tolower(fname[i]);
		if (letter == '.')
		{
			break;                          // don't include extension
		}
		if (letter == PATH_SEP)
		{
			letter = '/';                   // damn path names
		}
		hash += (long)(letter) * (i + 119);
		i++;
	}
	hash &= (FILE_HASH_SIZE - 1);
	return hash;
}

int GLSL_GetMacroByName(const char *name)
{
	int i;
	for(i = 0; i < MAX_MACROS; i++)
	{
		if (!Q_stricmp(name, complieMacroNames[i]))
		{
			return i;
		}
	}

	return -1;
}

void CopyStringAlloc(char **out,char *in)
{
	size_t size = strlen(in) * sizeof(char) + 1;
	*out = (char *) malloc(size);
	memset(*out,'\0',size);
	Q_strncpyz(*out, in, size);
}

qboolean CopyNextToken(char **text,char **out)
{
	char *token;
	token = COM_ParseExt(text, qtrue);
	if (!token[0])
	{
		return qfalse;
	}

	// end of shader definition
	if (token[0] == '}')
	{
		return qfalse;
	}

	CopyStringAlloc(out,token);
	return qtrue;
}

programInfo_t *GLSL_ParseDefinition(char **text,const char *defname)
{
	char *token;
	int  i = 0;
	programInfo_t *def;
	void *valptr;

	token = COM_ParseExt(text, qtrue);
	if (token[0] != '{')
	{
		ri.Printf(PRINT_WARNING, "WARNING: expecting '{', found '%s' instead in shader definition '%s'\n", token, defname);
		return NULL;
	}

	def = (programInfo_t *)malloc(sizeof(programInfo_t));
	memset(def,0,sizeof(programInfo_t));

	def->compiled = qfalse;

	while (1)
	{
		token = COM_ParseExt(text, qtrue);
		if (!token[0])
		{
			ri.Printf(PRINT_WARNING, "WARNING: no concluding '}' in shader definition %s\n", defname);
			goto parseerror;
		}

		// end of shader definition
		if (token[0] == '}')
		{
			break;
		}
		else if (!Q_stricmp(token, "filename"))
		{
			CopyNextToken(text,&def->filename);
		}
		else if (!Q_stricmp(token, "fragfilename"))
		{
			CopyNextToken(text,&def->fragFilename);
		}
		else if (!Q_stricmp(token, "macros"))
		{
			int macro;
			while((token = COM_ParseExt(text, qfalse))[0])
			{
				macro = GLSL_GetMacroByName(token);
				if(macro >= 0)
				{
					def->macros[def->numMacros] = macro;
					def->numMacros++;
				}
				else
				{
					ri.Printf(PRINT_WARNING,"WARNING: Macro '%s' for shaderdef '%s' was not recognized\n",token,defname);
					goto parseerror;
				}
			}
		}
		else if (!Q_stricmp(token, "extramacros"))
		{
			CopyNextToken(text,&def->extraMacros);
		}
		else if (!Q_stricmp(token, "vertexLibraries"))
		{
			CopyNextToken(text,&def->vertexLibraries);
		}
		else if (!Q_stricmp(token, "fragmentLibraries"))
		{
			CopyNextToken(text,&def->fragmentLibraries);
		}
		else if (!Q_stricmp(token, "uniform"))
		{
			token = COM_ParseExt(text, qtrue);
			if(!Q_stricmp(token, "int"))
			{
				def->uniformValues[def->numUniformValues].type.type = GLSL_INT;
				CopyNextToken(text,&def->uniformValues[def->numUniformValues].type.name);
				token = COM_ParseExt(text, qtrue);
				valptr = malloc(sizeof(int));
				*((int*)valptr) = atoi(token);
				def->uniformValues[def->numUniformValues].value = valptr;
				//Com_Printf("%d\n",*((int*)valptr));
			}
			//FIXME: implement other formats
			def->numUniformValues++;
		}
	}


	return def;

parseerror:
	free(def);
	return NULL;
}

static char *GLSL_FindDefinitionInText(const char *shadername)
{
	char *p = definitionText;
	char *token;

	// look for label
	// note that this could get confused if a shader name is used inside
	// another shader definition
	while (1)
	{
		token = COM_ParseExt(&p, qtrue);
		if (token[0] == 0)
		{
			break;
		}

		if (!Q_stricmp(token, shadername))
		{
			return p;
		}

		SkipBracedSection(&p);
	}

	return NULL;
}

programInfo_t *GLSL_FindShader(const char *name)
{
	char     strippedName[MAX_QPATH];
	char     fileName[MAX_QPATH];
	int      i, hash;
	char     *shaderText;
	programInfo_t *sh;

	if (name[0] == 0)
	{
		return DEFAULT_SHADER_DEF;
	}

	COM_StripExtension(name, strippedName, sizeof(strippedName));
	COM_FixPath(strippedName);

	hash = generateHashValue(strippedName);

	for (sh = hashTable[hash]; sh; sh = sh->next)
	{
		// index by name
		// the original way was correct
		if (!Q_stricmp(sh->name, strippedName))
		{
			// match found
			return sh;
		}
	}

	shaderText = GLSL_FindDefinitionInText(strippedName);
	if(!shaderText)
	{
		ri.Printf(PRINT_ALL,"Shader definition find failed: %s\n",name);
		return DEFAULT_SHADER_DEF;
	}

	sh = GLSL_ParseDefinition(&shaderText,strippedName);
	if(!sh)
	{
		ri.Printf(PRINT_ALL,"Shader definition parsing failed: %s\n",name);
		return DEFAULT_SHADER_DEF;
	}
	else
	{
		sh->next = hashTable[hash];
		hashTable[hash] = sh;
		return sh;
	}
}

void GLSL_LoadDefinitions(void)
{
	/*
	char **shaderFiles;
	char *buffers[MAX_SHADER_DEF_FILES];
	char *p;
	int  numShaderFiles;
	*/
	programInfo_t *test;
	programInfo_t *test2;

	//FIXME: Also load from external files in the future...
	//For no just copy the existing data to our searchable string
	definitionText = (char *)ri.Hunk_Alloc(strlen(defaultShaderDefinitions) + 1, h_low);
	memset(definitionText,'\0',strlen(defaultShaderDefinitions) + 1);
	Q_strncpyz(definitionText,defaultShaderDefinitions,strlen(defaultShaderDefinitions));
	

	//Test code
	/*
	test = GLSL_FindShader("vertexLighting_DBS_entity");

	ri.Printf(PRINT_ALL,"GLGL_LoadDefitionions end Filename %s Vert libs: %s\n",test->filename,test->vertexLibraries);

	test2 = GLSL_FindShader("generic");
	test = GLSL_FindShader("generic");

	ri.Printf(PRINT_ALL,"GLGL_LoadDefitionions end Filename %s Vert libs: %s\n",test->filename,test->vertexLibraries);

	ri.Error(ERR_FATAL,"GLGL_LoadDefitionions end Filename %s Vert libs: %s",test2->filename,test2->vertexLibraries);
	*/
}

#ifdef RENDERER2C

static void GLSL_PrintInfoLog(GLhandleARB object, qboolean developerOnly)
{
	char        *msg;
	static char msgPart[1024];
	int         maxLength = 0;
	int         i;
	int         printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_ALL;

	qglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

	if (maxLength <= 0)
	{
		ri.Printf(printLevel, "No compile log.\n");
		return;
	}

	ri.Printf(printLevel, "compile log:\n");

	if (maxLength < 1023)
	{
		qglGetInfoLogARB(object, maxLength, &maxLength, msgPart);

		msgPart[maxLength + 1] = '\0';

		ri.Printf(printLevel, "%s\n", msgPart);
	}
	else
	{
		msg = Ren_Malloc(maxLength);

		qglGetInfoLogARB(object, maxLength, &maxLength, msg);

		for (i = 0; i < maxLength; i += 1024)
		{
			Q_strncpyz(msgPart, msg + i, sizeof(msgPart));

			ri.Printf(printLevel, "%s\n", msgPart);
		}
		ri.Free(msg);
	}
}

static void GLSL_PrintShaderSource(GLhandleARB object)
{
	char        *msg;
	static char msgPart[1024];
	int         maxLength = 0;
	int         i;

	qglGetObjectParameterivARB(object, GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, &maxLength);

	msg = Ren_Malloc(maxLength);

	qglGetShaderSourceARB(object, maxLength, &maxLength, msg);

	for (i = 0; i < maxLength; i += 1024)
	{
		Q_strncpyz(msgPart, msg + i, sizeof(msgPart));
		ri.Printf(PRINT_ALL, "%s\n", msgPart);
	}

	ri.Free(msg);
}

static qboolean GLSL_HasConflictingMacros(int compilemacro, int usedmacros)
{
	switch (compilemacro)
	{
	case USE_VERTEX_SKINNING:
		if (usedmacros & USE_VERTEX_ANIMATION)
		{
			return qtrue;
		}
		break;
	case USE_DEFORM_VERTEXES:
		if (glConfig.driverType != GLDRV_OPENGL3 || !r_vboDeformVertexes->integer)
		{
			return qtrue;
		}
		break;
	case USE_VERTEX_ANIMATION:
		if (usedmacros & USE_VERTEX_SKINNING)
		{
			return qtrue;
		}
		break;
	default:
		return qfalse;
	}

	return qfalse;
}

static qboolean GLSL_MissesRequiredMacros(int compilemacro, unsigned long usedmacros)
{
	switch (compilemacro)
	{
	case USE_PARALLAX_MAPPING:
		if (usedmacros & USE_NORMAL_MAPPING)
		{
			return qtrue;
		}
		break;
	case USE_REFLECTIVE_SPECULAR:
		if (usedmacros & USE_NORMAL_MAPPING)
		{
			return qtrue;
		}
		break;
	case USE_VERTEX_SKINNING:
		if (!glConfig2.vboVertexSkinningAvailable)
		{
			return qtrue;
		}
		break;
	default:
		return qfalse;
	}
	return qfalse;
}

static unsigned int GLSL_GetRequiredVertexAttributes(int compilemacro)
{
	unsigned int attr = 0;

	switch (compilemacro)
	{
	case USE_VERTEX_ANIMATION:
		attr = ATTR_NORMAL | ATTR_POSITION2 | ATTR_NORMAL2;

		if (r_normalMapping->integer)
		{
			attr |= ATTR_TANGENT2 | ATTR_BINORMAL2;
		}
		break;
	case USE_VERTEX_SKINNING:
		attr = ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		break;
	case USE_DEFORM_VERTEXES:
		attr = ATTR_NORMAL;
		break;
	case USE_NORMAL_MAPPING:
		attr = ATTR_NORMAL | ATTR_TANGENT | ATTR_BINORMAL;
		break;
	case USE_TCGEN_ENVIRONMENT:
		attr = ATTR_NORMAL;
		break;
	case USE_TCGEN_LIGHTMAP:
		attr = ATTR_LIGHTCOORD;
		break;
	default:
		attr = 0;
		break;
	}

	return attr;
}

static void GLSL_GetShaderExtraDefines(char **defines, int *size)
{
	static char bufferExtra[32000];

	char *bufferFinal = NULL;
	int  sizeFinal;

	float fbufWidthScale, fbufHeightScale;
	float npotWidthScale, npotHeightScale;

	Com_Memset(bufferExtra, 0, sizeof(bufferExtra));

	// HACK: add some macros to avoid extra uniforms and save speed and code maintenance
	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef r_SpecularExponent\n#define r_SpecularExponent %f\n#endif\n", r_specularExponent->value));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef r_SpecularExponent2\n#define r_SpecularExponent2 %f\n#endif\n", r_specularExponent2->value));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef r_SpecularScale\n#define r_SpecularScale %f\n#endif\n", r_specularScale->value));
	//Q_strcat(bufferExtra, sizeof(bufferExtra),
	//       va("#ifndef r_NormalScale\n#define r_NormalScale %f\n#endif\n", r_normalScale->value));

	Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef M_PI\n#define M_PI 3.14159265358979323846f\n#endif\n");

	Q_strcat(bufferExtra, sizeof(bufferExtra), va("#ifndef MAX_SHADOWMAPS\n#define MAX_SHADOWMAPS %i\n#endif\n", MAX_SHADOWMAPS));

	Q_strcat(bufferExtra, sizeof(bufferExtra), va("#ifndef MAX_SHADER_DEFORM_PARMS\n#define MAX_SHADER_DEFORM_PARMS %i\n#endif\n", MAX_SHADER_DEFORM_PARMS));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef deform_t\n"
	            "#define deform_t\n"
	            "#define DEFORM_WAVE %i\n"
	            "#define DEFORM_BULGE %i\n"
	            "#define DEFORM_MOVE %i\n"
	            "#endif\n",
	            DEFORM_WAVE,
	            DEFORM_BULGE,
	            DEFORM_MOVE));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef genFunc_t\n"
	            "#define genFunc_t\n"
	            "#define GF_NONE %1.1f\n"
	            "#define GF_SIN %1.1f\n"
	            "#define GF_SQUARE %1.1f\n"
	            "#define GF_TRIANGLE %1.1f\n"
	            "#define GF_SAWTOOTH %1.1f\n"
	            "#define GF_INVERSE_SAWTOOTH %1.1f\n"
	            "#define GF_NOISE %1.1f\n"
	            "#endif\n",
	            ( float ) GF_NONE,
	            ( float ) GF_SIN,
	            ( float ) GF_SQUARE,
	            ( float ) GF_TRIANGLE,
	            ( float ) GF_SAWTOOTH,
	            ( float ) GF_INVERSE_SAWTOOTH,
	            ( float ) GF_NOISE));

	/*
	Q_strcat(bufferExtra, sizeof(bufferExtra),
	                                 va("#ifndef deformGen_t\n"
	                                        "#define deformGen_t\n"
	                                        "#define DGEN_WAVE_SIN %1.1f\n"
	                                        "#define DGEN_WAVE_SQUARE %1.1f\n"
	                                        "#define DGEN_WAVE_TRIANGLE %1.1f\n"
	                                        "#define DGEN_WAVE_SAWTOOTH %1.1f\n"
	                                        "#define DGEN_WAVE_INVERSE_SAWTOOTH %1.1f\n"
	                                        "#define DGEN_BULGE %i\n"
	                                        "#define DGEN_MOVE %i\n"
	                                        "#endif\n",
	                                        (float)DGEN_WAVE_SIN,
	                                        (float)DGEN_WAVE_SQUARE,
	                                        (float)DGEN_WAVE_TRIANGLE,
	                                        (float)DGEN_WAVE_SAWTOOTH,
	                                        (float)DGEN_WAVE_INVERSE_SAWTOOTH,
	                                        DGEN_BULGE,
	                                        DGEN_MOVE));
	                                */

	/*
	Q_strcat(bufferExtra, sizeof(bufferExtra),
	                                 va("#ifndef colorGen_t\n"
	                                        "#define colorGen_t\n"
	                                        "#define CGEN_VERTEX %i\n"
	                                        "#define CGEN_ONE_MINUS_VERTEX %i\n"
	                                        "#endif\n",
	                                        CGEN_VERTEX,
	                                        CGEN_ONE_MINUS_VERTEX));

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	                                                 va("#ifndef alphaGen_t\n"
	                                                        "#define alphaGen_t\n"
	                                                        "#define AGEN_VERTEX %i\n"
	                                                        "#define AGEN_ONE_MINUS_VERTEX %i\n"
	                                                        "#endif\n",
	                                                        AGEN_VERTEX,
	                                                        AGEN_ONE_MINUS_VERTEX));
	                                                        */

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef alphaTest_t\n"
	            "#define alphaTest_t\n"
	            "#define ATEST_GT_0 %i\n"
	            "#define ATEST_LT_128 %i\n"
	            "#define ATEST_GE_128 %i\n"
	            "#endif\n",
	            ATEST_GT_0,
	            ATEST_LT_128,
	            ATEST_GE_128));

	fbufWidthScale  = Q_recip(( float ) glConfig.vidWidth);
	fbufHeightScale = Q_recip(( float ) glConfig.vidHeight);
	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef r_FBufScale\n#define r_FBufScale vec2(%f, %f)\n#endif\n", fbufWidthScale, fbufHeightScale));

	if (glConfig2.textureNPOTAvailable)
	{
		npotWidthScale  = 1;
		npotHeightScale = 1;
	}
	else
	{
		npotWidthScale  = ( float ) glConfig.vidWidth / ( float ) NearestPowerOfTwo(glConfig.vidWidth);
		npotHeightScale = ( float ) glConfig.vidHeight / ( float ) NearestPowerOfTwo(glConfig.vidHeight);
	}

	Q_strcat(bufferExtra, sizeof(bufferExtra),
	         va("#ifndef r_NPOTScale\n#define r_NPOTScale vec2(%f, %f)\n#endif\n", npotWidthScale, npotHeightScale));

	if (glConfig.driverType == GLDRV_MESA)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLDRV_MESA\n#define GLDRV_MESA 1\n#endif\n");
	}

	if (glConfig.hardwareType == GLHW_ATI)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLHW_ATI\n#define GLHW_ATI 1\n#endif\n");
	}
	else if (glConfig.hardwareType == GLHW_ATI_DX10)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLHW_ATI_DX10\n#define GLHW_ATI_DX10 1\n#endif\n");
	}
	else if (glConfig.hardwareType == GLHW_NV_DX10)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GLHW_NV_DX10\n#define GLHW_NV_DX10 1\n#endif\n");
	}

	if (r_shadows->integer >= SHADOWING_ESM16 && glConfig2.textureFloatAvailable && glConfig2.framebufferObjectAvailable)
	{
		if (r_shadows->integer == SHADOWING_ESM16 || r_shadows->integer == SHADOWING_ESM32)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef ESM\n#define ESM 1\n#endif\n");
		}
		else if (r_shadows->integer == SHADOWING_EVSM32)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef EVSM\n#define EVSM 1\n#endif\n");

			// The exponents for the EVSM techniques should be less than ln(FLT_MAX/FILTER_SIZE)/2 {ln(FLT_MAX/1)/2 ~44.3}
			//         42.9 is the maximum possible value for FILTER_SIZE=15
			//         42.0 is the truncated value that we pass into the sample
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_EVSMExponents\n#define r_EVSMExponents vec2(%f, %f)\n#endif\n", 42.0f, 42.0f));

			if (r_evsmPostProcess->integer)
			{
				Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_EVSMPostProcess\n#define r_EVSMPostProcess 1\n#endif\n");
			}
		}
		else
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM\n#define VSM 1\n#endif\n");

			if (glConfig.hardwareType == GLHW_ATI)
			{
				Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM_CLAMP\n#define VSM_CLAMP 1\n#endif\n");
			}
		}

		if ((glConfig.hardwareType == GLHW_NV_DX10 || glConfig.hardwareType == GLHW_ATI_DX10) && r_shadows->integer == SHADOWING_VSM32)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM_EPSILON\n#define VSM_EPSILON 0.000001\n#endif\n");
		}
		else
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef VSM_EPSILON\n#define VSM_EPSILON 0.0001\n#endif\n");
		}

		if (r_lightBleedReduction->value)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_LightBleedReduction\n#define r_LightBleedReduction %f\n#endif\n",
			            r_lightBleedReduction->value));
		}

		if (r_overDarkeningFactor->value)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_OverDarkeningFactor\n#define r_OverDarkeningFactor %f\n#endif\n",
			            r_overDarkeningFactor->value));
		}

		if (r_shadowMapDepthScale->value)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_ShadowMapDepthScale\n#define r_ShadowMapDepthScale %f\n#endif\n",
			            r_shadowMapDepthScale->value));
		}

		if (r_debugShadowMaps->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_DebugShadowMaps\n#define r_DebugShadowMaps %i\n#endif\n", r_debugShadowMaps->integer));
		}

		/*
		if(r_softShadows->integer == 1)
		{
		        Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCF_2X2\n#define PCF_2X2 1\n#endif\n");
		}
		else if(r_softShadows->integer == 2)
		{
		        Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCF_3X3\n#define PCF_3X3 1\n#endif\n");
		}
		else if(r_softShadows->integer == 3)
		{
		        Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCF_4X4\n#define PCF_4X4 1\n#endif\n");
		}
		else if(r_softShadows->integer == 4)
		{
		        Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCF_5X5\n#define PCF_5X5 1\n#endif\n");
		}
		else if(r_softShadows->integer == 5)
		{
		        Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCF_6X6\n#define PCF_6X6 1\n#endif\n");
		}
		*/
		if (r_softShadows->integer == 6)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef PCSS\n#define PCSS 1\n#endif\n");
		}
		else if (r_softShadows->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_PCFSamples\n#define r_PCFSamples %1.1f\n#endif\n", r_softShadows->value + 1.0f));
		}

		if (r_parallelShadowSplits->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra),
			         va("#ifndef r_ParallelShadowSplits_%i\n#define r_ParallelShadowSplits_%i\n#endif\n", r_parallelShadowSplits->integer, r_parallelShadowSplits->integer));
		}

		if (r_showParallelShadowSplits->integer)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_ShowParallelShadowSplits\n#define r_ShowParallelShadowSplits 1\n#endif\n");
		}
	}

	if (r_deferredShading->integer && glConfig2.maxColorAttachments >= 4 && glConfig2.textureFloatAvailable &&
	    glConfig2.drawBuffersAvailable && glConfig2.maxDrawBuffers >= 4)
	{
		if (r_deferredShading->integer == DS_STANDARD)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_DeferredShading\n#define r_DeferredShading 1\n#endif\n");
		}
	}

	if (r_hdrRendering->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_HDRRendering\n#define r_HDRRendering 1\n#endif\n");

		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef r_HDRContrastThreshold\n#define r_HDRContrastThreshold %f\n#endif\n",
		            r_hdrContrastThreshold->value));

		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef r_HDRContrastOffset\n#define r_HDRContrastOffset %f\n#endif\n",
		            r_hdrContrastOffset->value));

		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef r_HDRToneMappingOperator\n#define r_HDRToneMappingOperator_%i\n#endif\n",
		            r_hdrToneMappingOperator->integer));

		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef r_HDRGamma\n#define r_HDRGamma %f\n#endif\n",
		            r_hdrGamma->value));
	}

	if (r_precomputedLighting->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         "#ifndef r_precomputedLighting\n#define r_precomputedLighting 1\n#endif\n");
	}

	if (r_heatHazeFix->integer && glConfig2.framebufferBlitAvailable && /*glConfig.hardwareType != GLHW_ATI && glConfig.hardwareType != GLHW_ATI_DX10 &&*/ glConfig.driverType != GLDRV_MESA)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_heatHazeFix\n#define r_heatHazeFix 1\n#endif\n");
	}

	if (r_showLightMaps->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_showLightMaps\n#define r_showLightMaps 1\n#endif\n");
	}

	if (r_showDeluxeMaps->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_showDeluxeMaps\n#define r_showDeluxeMaps 1\n#endif\n");
	}

#ifdef EXPERIMENTAL

	if (r_screenSpaceAmbientOcclusion->integer)
	{
		int             i;
		static vec3_t   jitter[32];
		static qboolean jitterInit = qfalse;

		if (!jitterInit)
		{
			for (i = 0; i < 32; i++)
			{
				float *jit = &jitter[i][0];

				float rad = crandom() * 1024.0f;     // FIXME radius;
				float a   = crandom() * M_PI * 2;
				float b   = crandom() * M_PI * 2;

				jit[0] = rad * sin(a) * cos(b);
				jit[1] = rad * sin(a) * sin(b);
				jit[2] = rad * cos(a);
			}

			jitterInit = qtrue;
		}

		// TODO
	}

#endif

	if (glConfig2.vboVertexSkinningAvailable)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_VertexSkinning\n#define r_VertexSkinning 1\n#endif\n");

		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef MAX_GLSL_BONES\n#define MAX_GLSL_BONES %i\n#endif\n", glConfig2.maxVertexSkinningBones));
	}
	else
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef MAX_GLSL_BONES\n#define MAX_GLSL_BONES %i\n#endif\n", 4));
	}

	/*
	   if(glConfig.drawBuffersAvailable && glConfig.maxDrawBuffers >= 4)
	   {
	   //Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GL_ARB_draw_buffers\n#define GL_ARB_draw_buffers 1\n#endif\n");
	   Q_strcat(bufferExtra, sizeof(bufferExtra), "#extension GL_ARB_draw_buffers : enable\n");
	   }
	 */

	if (r_normalMapping->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_NormalMapping\n#define r_NormalMapping 1\n#endif\n");
	}

	if (/* TODO: check for shader model 3 hardware  && */ r_normalMapping->integer && r_parallaxMapping->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_ParallaxMapping\n#define r_ParallaxMapping 1\n#endif\n");
	}

	if (r_wrapAroundLighting->value)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra),
		         va("#ifndef r_WrapAroundLighting\n#define r_WrapAroundLighting %f\n#endif\n",
		            r_wrapAroundLighting->value));
	}

	if (r_halfLambertLighting->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_HalfLambertLighting\n#define r_HalfLambertLighting 1\n#endif\n");
	}

	if (r_rimLighting->integer)
	{
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_RimLighting\n#define r_RimLighting 1\n#endif\n");
		Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef r_RimColor\n#define r_RimColor vec4(0.26, 0.19, 0.16, 0.0)\n#endif\n");
		Q_strcat(bufferExtra, sizeof(bufferExtra), va("#ifndef r_RimExponent\n#define r_RimExponent %f\n#endif\n",
		                                              r_rimExponent->value));
	}

	// OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	Q_strcat(bufferExtra, sizeof(bufferExtra), "#line 0\n");

	*size    = strlen(bufferExtra) + 1;
	*defines = (char *) malloc(*size);
	memset(*defines, 0, *size);
	Q_strcat(*defines, *size, bufferExtra);
}

static void GLSL_GetShaderHeader(GLenum shaderType, char *dest, int size)
{
	dest[0] = '\0';

	// HACK: abuse the GLSL preprocessor to turn GLSL 1.20 shaders into 1.30 ones
	if (glRefConfig.glslMajorVersion > 1 || (glRefConfig.glslMajorVersion == 1 && glRefConfig.glslMinorVersion >= 30))
	{
		Q_strcat(dest, size, "#version 130\n");

		if (shaderType == GL_VERTEX_SHADER)
		{
			Q_strcat(dest, size, "#define attribute in\n");
			Q_strcat(dest, size, "#define varying out\n");
		}
		else
		{
			Q_strcat(dest, size, "#define varying in\n");

			Q_strcat(dest, size, "out vec4 out_Color;\n");
			Q_strcat(dest, size, "#define gl_FragColor out_Color\n");
		}

		Q_strcat(dest, size, "#define textureCube texture\n");
	}
	else
	{
		Q_strcat(dest, size, "#version 120\n");
	}
}

static int GLSL_CompileGPUShader(GLhandleARB program, GLhandleARB *prevShader, const GLcharARB *buffer, int size, GLenum shaderType)
{
	GLint       compiled;
	GLhandleARB shader;

	shader = qglCreateShaderObjectARB(shaderType);

	qglShaderSourceARB(shader, 1, (const GLcharARB **)&buffer, &size);

	// compile shader
	qglCompileShaderARB(shader);

	// check if shader compiled
	qglGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (!compiled)
	{
		GLSL_PrintShaderSource(shader);
		GLSL_PrintInfoLog(shader, qfalse);
		ri.Error(ERR_DROP, "Couldn't compile shader");
		return 0;
	}

	//GLSL_PrintInfoLog(shader, qtrue);
	//GLSL_PrintShaderSource(shader);

	if (*prevShader)
	{
		qglDetachObjectARB(program, *prevShader);
		qglDeleteObjectARB(*prevShader);
	}

	// attach shader to program
	qglAttachObjectARB(program, shader);

	*prevShader = shader;

	return 1;
}

static void GLSL_GetShaderText(const char *name, GLenum shaderType, char **data, int *size, qboolean append)
{
	char fullname[MAX_QPATH];
	int  dataSize;
	char *dataBuffer;

	if (shaderType == GL_VERTEX_SHADER)
	{
		Com_sprintf(fullname, sizeof(fullname), "%s_vp", name);
		ri.Printf(PRINT_ALL, "...loading vertex shader '%s'\n", fullname);
	}
	else
	{
		Com_sprintf(fullname, sizeof(fullname), "%s_fp", name);
		ri.Printf(PRINT_ALL, "...loading vertex shader '%s'\n", fullname);
	}

	if (ri.FS_FOpenFileRead(va("glsl/%s.glsl", fullname), NULL, qfalse))
	{
		dataSize = ri.FS_ReadFile(va("glsl/%s.glsl", fullname), ( void ** ) &dataBuffer);
	}
	else
	{
		dataBuffer = NULL;
	}

	if (!dataBuffer)
	{
		const char *temp = NULL;

		temp = GetFallbackShader(fullname);
		if (temp)
		{
			//Found a fallback shader and will use it
			int strl = 0;
			strl = strlen(temp) + 1;
			if (append && *size)
			{
				*data = ( char * ) realloc(*data, *size + strl);
				memset(*data + *size, 0, strl);

			}
			else
			{
				*data = (char *) malloc(strl);
				memset(*data, 0, strl);
			}

			*size += strl;

			Q_strcat(*data, *size, temp);
			Q_strcat(*data, *size, "\n");
		}
		else
		{
			ri.Error(ERR_FATAL, "Couldn't load shader %s", fullname);
		}
	}
	else
	{
		++dataSize; //We incease this for the newline
		if (append && *size)
		{
			*data = ( char * ) realloc(*data, *size + dataSize);
			memset(*data + *size, 0, dataSize);
		}
		else
		{
			*data = (char *) malloc(dataSize);
			memset(*data, 0, dataSize);
		}

		*size += dataSize;

		Q_strcat(*data, *size, dataBuffer);
		Q_strcat(*data, *size, "\n");
	}

	if (dataBuffer)
	{
		ri.FS_FreeFile(dataBuffer);
	}

	Com_Printf("Loaded shader '%s'\n", fullname);
}

static char *GLSL_BuildGPUShaderText(const char *mainShaderName, const char *libShaderNames, GLenum shaderType)
{
	GLchar *mainBuffer = NULL;
	int    mainSize    = 0;
	char   *token;

	int  libsSize    = 0;
	char *libsBuffer = NULL;        // all libs concatenated

	char **libs = ( char ** ) &libShaderNames;

	char *shaderText = NULL;

	GL_CheckErrors();

	while (1)
	{
		token = COM_ParseExt2(libs, qfalse);

		if (!token[0])
		{
			break;
		}
		GLSL_GetShaderText(token, shaderType, &libsBuffer, &libsSize, qtrue);
	}

	// load main() program
	GLSL_GetShaderText(mainShaderName, shaderType, &mainBuffer, &mainSize, qfalse);

	if (!libsBuffer && !mainBuffer)
	{
		ri.Error(ERR_FATAL, "Shader loading failed!\n");
	}
	{
		char *shaderExtra = NULL;
		int  extraSize    = 0;

		char *bufferFinal = NULL;
		int  sizeFinal;

		GLSL_GetShaderExtraDefines(&shaderExtra, &extraSize);

		sizeFinal = extraSize + mainSize + libsSize;

		bufferFinal = ( char * ) ri.Hunk_AllocateTempMemory(sizeFinal);

		strcpy(bufferFinal, shaderExtra);

		if (libsSize > 0)
		{
			Q_strcat(bufferFinal, sizeFinal, libsBuffer);
		}

		Q_strcat(bufferFinal, sizeFinal, mainBuffer);

		shaderText = malloc(sizeFinal);
		strcpy(shaderText, bufferFinal);
		ri.Hunk_FreeTempMemory(bufferFinal);
		free(shaderExtra);
	}
	free(mainBuffer);
	free(libsBuffer);

	return shaderText;
}

/*
This whole method is stupid, clean this shit up
*/
static qboolean GLSL_GenerateMacroString(shaderProgramList_t *program, const char *macros, int permutation, char **out)
{
	int i;
	unsigned long macroatrib = 0;

	*out = (char *) malloc(1000);
	memset(*out, 0, 1000);

	if (permutation)
	{
		for (i = 0; i < program->mappedMacros; i++)
		{
			if (permutation & BIT(program->macromap[i].bitOffset))
			{
				macroatrib |= BIT(program->macromap[i].macro);
			}
		}

		for (i = 0; i < MAX_MACROS; i++)
		{
			if (macroatrib & BIT(i))
			{
				if (GLSL_HasConflictingMacros(i, macroatrib))
				{
					return qfalse;
				}

				if (GLSL_MissesRequiredMacros(i, macroatrib))
				{
					return qfalse;
				}

				Q_strcat(*out, 1000, va("%s ", complieMacroNames[i]));
			}
		}
	}

	if (macros)
	{
		Q_strcat(*out, 1000, macros);
	}

	return qtrue;
}

static void GLSL_LinkProgram(GLhandleARB program)
{
	GLint linked;

	qglLinkProgramARB(program);

	qglGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if (!linked)
	{
		GLSL_PrintInfoLog(program, qfalse);
		ri.Printf(PRINT_ALL, "\n");
		ri.Error(ERR_DROP, "shaders failed to link");
	}
}

static void GLSL_ValidateProgram(GLhandleARB program)
{
	GLint validated;

	qglValidateProgramARB(program);

	qglGetObjectParameterivARB(program, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if (!validated)
	{
		GLSL_PrintInfoLog(program, qfalse);
		ri.Printf(PRINT_ALL, "\n");
		ri.Error(ERR_DROP, "shaders failed to validate");
	}
}

static void GLSL_ShowProgramUniforms(GLhandleARB program)
{
	int    i, count, size;
	GLenum type;
	char   uniformName[1000];

	// install the executables in the program object as part of current state.
	qglUseProgramObjectARB(program);

	// check for GL Errors

	// query the number of active uniforms
	qglGetObjectParameterivARB(program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &count);

	// Loop over each of the active uniforms, and set their value
	for (i = 0; i < count; i++)
	{
		qglGetActiveUniformARB(program, i, sizeof(uniformName), NULL, &size, &type, uniformName);

		ri.Printf(PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName);
	}

	qglUseProgramObjectARB(0);
}

void GLSL_InitUniforms(shaderProgram_t *program)
{
	int i, size;

	GLint *uniforms = program->uniforms;

	size = 0;
	for (i = 0; i < UNIFORM_COUNT; i++)
	{
		uniforms[i] = qglGetUniformLocationARB(program->program, uniformsInfo[i].name);

		if (uniforms[i] == -1)
		{
			continue;
		}

		program->uniformBufferOffsets[i] = size;

		switch (uniformsInfo[i].type)
		{
		case GLSL_INT:
			size += sizeof(GLint);
			break;
		case GLSL_FLOAT:
			size += sizeof(GLfloat);
			break;
		case GLSL_FLOAT5:
			size += sizeof(vec_t) * 5;
			break;
		case GLSL_VEC2:
			size += sizeof(vec_t) * 2;
			break;
		case GLSL_VEC3:
			size += sizeof(vec_t) * 3;
			break;
		case GLSL_VEC4:
			size += sizeof(vec_t) * 4;
			break;
		case GLSL_MAT16:
			size += sizeof(vec_t) * 16;
			break;
		default:
			break;
		}
	}

	program->uniformBuffer = Ren_Malloc(size);
}

void GLSL_FinishGPUShader(shaderProgram_t *program)
{
	GLSL_ValidateProgram(program->program);
	GLSL_ShowProgramUniforms(program->program);
	GL_CheckErrors();
}

void GLSL_SetUniformInt(shaderProgram_t *program, int uniformNum, GLint value)
{
	GLint *uniforms = program->uniforms;
	GLint *compare  = (GLint *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_INT)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformInt: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (value == *compare)
	{
		return;
	}

	*compare = value;

	qglUniform1iARB(uniforms[uniformNum], value);
}

void GLSL_SetUniformFloat(shaderProgram_t *program, int uniformNum, GLfloat value)
{
	GLint   *uniforms = program->uniforms;
	GLfloat *compare  = (GLfloat *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_FLOAT)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformFloat: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (value == *compare)
	{
		return;
	}

	*compare = value;

	qglUniform1fARB(uniforms[uniformNum], value);
}

void GLSL_SetUniformVec2(shaderProgram_t *program, int uniformNum, const vec2_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare  = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_VEC2)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformVec2: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (v[0] == compare[0] && v[1] == compare[1])
	{
		return;
	}

	compare[0] = v[0];
	compare[1] = v[1];

	qglUniform2fARB(uniforms[uniformNum], v[0], v[1]);
}

void GLSL_SetUniformVec3(shaderProgram_t *program, int uniformNum, const vec3_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare  = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_VEC3)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformVec3: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (VectorCompare(v, compare))
	{
		return;
	}

	VectorCopy(v, compare);

	qglUniform3fARB(uniforms[uniformNum], v[0], v[1], v[2]);
}

void GLSL_SetUniformVec4(shaderProgram_t *program, int uniformNum, const vec4_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare  = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_VEC4)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformVec4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (Vector4Compare(v, compare))
	{
		return;
	}

	Vector4Copy(v, compare);

	qglUniform4fARB(uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void GLSL_SetUniformFloat5(shaderProgram_t *program, int uniformNum, const vec5_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare  = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_FLOAT5)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformFloat5: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (Vector5Compare(v, compare))
	{
		return;
	}

	Vector5Copy(v, compare);

	qglUniform1fvARB(uniforms[uniformNum], 5, v);
}

void GLSL_SetUniformMatrix16(shaderProgram_t *program, int uniformNum, const matrix_t matrix)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare  = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
	{
		return;
	}

	if (uniformsInfo[uniformNum].type != GLSL_MAT16)
	{
		ri.Printf(PRINT_WARNING, "GLSL_SetUniformMatrix16: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (MatrixCompare(matrix, compare))
	{
		return;
	}

	MatrixCopy(matrix, compare);

	qglUniformMatrix4fvARB(uniforms[uniformNum], 1, GL_FALSE, matrix);
}

void GLSL_SetUniformVec4ARR(shaderProgram_t *program, int uniformNum, vec4_t *vectorarray,int arraysize)
{

}

void GLSL_SetUniformMatrix16ARR(shaderProgram_t *program, int uniformNum, matrix_t *matrixarray,int arraysize)
{

}

static void GLSL_BindAttribLocations(GLuint program)
{
	//if(attribs & ATTR_POSITION)
	glBindAttribLocation(program, ATTR_INDEX_POSITION, "attr_Position");

	//if(attribs & ATTR_TEXCOORD)
	glBindAttribLocation(program, ATTR_INDEX_TEXCOORD0, "attr_TexCoord0");

	//if(attribs & ATTR_LIGHTCOORD)
	glBindAttribLocation(program, ATTR_INDEX_TEXCOORD1, "attr_TexCoord1");

	//  if(attribs & ATTR_TEXCOORD2)
	//      glBindAttribLocation(program, ATTR_INDEX_TEXCOORD2, "attr_TexCoord2");

	//  if(attribs & ATTR_TEXCOORD3)
	//      glBindAttribLocation(program, ATTR_INDEX_TEXCOORD3, "attr_TexCoord3");

	//if(attribs & ATTR_TANGENT)
	glBindAttribLocation(program, ATTR_INDEX_TANGENT, "attr_Tangent");

	//if(attribs & ATTR_BINORMAL)
	glBindAttribLocation(program, ATTR_INDEX_BINORMAL, "attr_Binormal");

	//if(attribs & ATTR_NORMAL)
	glBindAttribLocation(program, ATTR_INDEX_NORMAL, "attr_Normal");

	//if(attribs & ATTR_COLOR)
	glBindAttribLocation(program, ATTR_INDEX_COLOR, "attr_Color");

	//if(glConfig2.vboVertexSkinningAvailable)
	{
		glBindAttribLocation(program, ATTR_INDEX_BONE_INDEXES, "attr_BoneIndexes");
		glBindAttribLocation(program, ATTR_INDEX_BONE_WEIGHTS, "attr_BoneWeights");
	}

	//if(attribs & ATTR_POSITION2)
	glBindAttribLocation(program, ATTR_INDEX_POSITION2, "attr_Position2");

	//if(attribs & ATTR_TANGENT2)
	glBindAttribLocation(program, ATTR_INDEX_TANGENT2, "attr_Tangent2");

	//if(attribs & ATTR_BINORMAL2)
	glBindAttribLocation(program, ATTR_INDEX_BINORMAL2, "attr_Binormal2");

	//if(attribs & ATTR_NORMAL2)
	glBindAttribLocation(program, ATTR_INDEX_NORMAL2, "attr_Normal2");
}

static qboolean GLSL_InitGPUShader2(shaderProgram_t *program, const char *name, const char *vpCode, const char *fpCode)
{
	ri.Printf(PRINT_DEVELOPER, "------- GPU shader -------\n");

	if (strlen(name) >= MAX_QPATH)
	{
		ri.Error(ERR_DROP, "GLSL_InitGPUShader2: \"%s\" is too long", name);
	}

	Q_strncpyz(program->name, name, sizeof(program->name));

	program->program = qglCreateProgramObjectARB();

	if (!(GLSL_CompileGPUShader(program->program, &program->vertexShader, vpCode, strlen(vpCode), GL_VERTEX_SHADER_ARB)))
	{
		ri.Printf(PRINT_ALL, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_VERTEX_SHADER_ARB\n", name);
		qglDeleteObjectARB(program->program);
		return qfalse;
	}

	if (fpCode)
	{
		if (!(GLSL_CompileGPUShader(program->program, &program->fragmentShader, fpCode, strlen(fpCode), GL_FRAGMENT_SHADER_ARB)))
		{
			ri.Printf(PRINT_ALL, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_FRAGMENT_SHADER_ARB\n", name);
			qglDeleteObjectARB(program->program);
			return qfalse;
		}
	}

	GLSL_BindAttribLocations(program->program);

	GLSL_LinkProgram(program->program);

	return qtrue;
}

static qboolean GLSL_FinnishShaderTextAndCompile(shaderProgram_t *program, const char *name, const char *vertex, const char *frag, const char *macrostring)
{
	char vpSource[32000];
	char fpSource[32000];
	int  size = sizeof(vpSource);

	GLSL_GetShaderHeader(GL_VERTEX_SHADER, vpSource, size);
	GLSL_GetShaderHeader(GL_FRAGMENT_SHADER, fpSource, size);

	if (macrostring)
	{
		char **compileMacrosP = ( char ** ) &macrostring;
		char *token;

		while (1)
		{
			token = COM_ParseExt2(compileMacrosP, qfalse);

			if (!token[0])
			{
				break;
			}

			Q_strcat(vpSource, size, va("#ifndef %s\n#define %s 1\n#endif\n", token, token));
			Q_strcat(fpSource, size, va("#ifndef %s\n#define %s 1\n#endif\n", token, token));
		}
	}

	Q_strcat(vpSource, size, vertex);
	Q_strcat(fpSource, size, frag);

	if(GLSL_InitGPUShader2(program, name, vpSource, fpSource))
	{
		GLSL_InitUniforms(program);
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

static void GLSL_SetInitialUniformValues(programInfo_t *info,int permutation)
{
	int i,location;
	GLSL_BindProgram(&info->list->programs[permutation]);

	for(i = 0; i < info->numUniformValues; i++)
	{
		location = qglGetUniformLocationARB(info->list->programs[permutation].program, info->uniformValues[i].type.name);

		switch (info->uniformValues[i].type.type)
		{
		case GLSL_INT:
			GLSL_SetUniformInt(&info->list->programs[permutation],location,*((int *)info->uniformValues[i].value));
			break;
		default:
			ri.Error(ERR_FATAL,"Only INT supported atm");
		}
	}

	GLSL_BindNullProgram();
}

static void GLSL_MapMacro(macroBitMap_t *map, int macro, int mappedbit)
{
	map->macro     = macro;
	map->bitOffset = mappedbit;
}

qboolean GLSL_CompileShaderList(programInfo_t *info)
{
	char   *vertexShader   = GLSL_BuildGPUShaderText(info->filename, info->vertexLibraries, GL_VERTEX_SHADER);
	char   *fragmentShader = GLSL_BuildGPUShaderText((info->fragFilename?info->fragFilename:info->filename), info->fragmentLibraries, GL_FRAGMENT_SHADER);
	int    macronum        = 0;
	int    startTime, endTime;
	size_t numPermutations = 0, numCompiled = 0, tics = 0, nextTicCount = 0;
	int    i               = 0;

	info->list = (shaderProgramList_t *)Ren_Malloc(sizeof(shaderProgramList_t));
	memset(info->list,0,sizeof(shaderProgramList_t));

	if(info->numMacros > 0)
	{
		info->list->macromap = Com_Allocate(sizeof(macroBitMap_t) * macronum);
		for(i = 0; i < info->numMacros; i++)
		{
			GLSL_MapMacro(&info->list->macromap[i], info->macros[i], i);
		}
		info->list->mappedMacros = i;
	}
	else
	{
		info->list->macromap     = NULL;
		info->list->mappedMacros = 0;
	}

	numPermutations = BIT(info->numMacros);

	ri.Printf(PRINT_ALL, "...compiling %s shaders\n", info->name);
	ri.Printf(PRINT_ALL, "0%%  10   20   30   40   50   60   70   80   90   100%%\n");
	ri.Printf(PRINT_ALL, "|----|----|----|----|----|----|----|----|----|----|\n");

	info->list->programs = Com_Allocate(sizeof(shaderProgram_t) * numPermutations);

	for (i = 0; i < numPermutations; i++)
	{
		char *tempString = NULL;

		if ((i + 1) >= nextTicCount)
		{
			size_t ticsNeeded = (size_t)(((double)(i + 1) / numPermutations) * 50.0);

			do
			{
				ri.Printf(PRINT_ALL, "*");
			}
			while (++tics < ticsNeeded);

			nextTicCount = (size_t)((tics / 50.0) * numPermutations);

			if (i == (numPermutations - 1))
			{
				if (tics < 51)
				{
					ri.Printf(PRINT_ALL, "*");
				}

				ri.Printf(PRINT_ALL, "\n");
			}
		}

		if (GLSL_GenerateMacroString(info->list, info->extraMacros, i, &tempString))
		{
			if(GLSL_FinnishShaderTextAndCompile(&info->list->programs[i], info->name, vertexShader, fragmentShader, tempString))
			{
				//Set uniform values
				GLSL_SetInitialUniformValues(info,i);
				GLSL_FinishGPUShader(&info->list->programs[i]);
			}

			numCompiled++;
		}
		else
		{
			info->list->programs[i].program = NULL;
		}

	}

	endTime = ri.Milliseconds();
	ri.Printf(PRINT_ALL, "...compiled %i %s shader permutations in %5.2f seconds\n", ( int ) numCompiled, info->name, (endTime - startTime) / 1000.0);
	info->compiled = qtrue;
	return qtrue;
}

programInfo_t *GLSL_GetShaderProgram(const char *name)
{
	programInfo_t *prog;

	prog = GLSL_FindShader(name);

	if(prog)
	{
		//Compile the shader program
		GLSL_CompileShaderList(prog);
	}

	return prog;
}

void GLSL_SetMacros(shaderProgramList_t *programlist, int macros)
{
	programlist->currentMacros = macros;
}

void GLSL_SetMacroState(programInfo_t *programlist,int macro,int enabled)
{
	//FIXME: implement this
}

void GLSL_SelectPermutation(programInfo_t *programlist)
{
	//FIXME: implement this
	//set the tr.selectedProgram
}

void GLSL_DeleteGPUShader(shaderProgram_t *program)
{
	if (program->program)
	{
		if (program->vertexShader)
		{
			qglDetachObjectARB(program->program, program->vertexShader);
			qglDeleteObjectARB(program->vertexShader);
		}

		if (program->fragmentShader)
		{
			qglDetachObjectARB(program->program, program->fragmentShader);
			qglDeleteObjectARB(program->fragmentShader);
		}

		qglDeleteObjectARB(program->program);

		if (program->uniformBuffer)
		{
			ri.Free(program->uniformBuffer);
		}

		Com_Memset(program, 0, sizeof(*program));
	}
}

void GLSL_InitGPUShaders(void)
{
	int  startTime, endTime;
	int  i;
	char extradefines[1024];
	int  attribs;
	int  numGenShaders = 0, numLightShaders = 0, numEtcShaders = 0;

	ri.Printf(PRINT_ALL, "------- GLSL_InitGPUShaders -------\n");

	R_IssuePendingRenderCommands();

	startTime = ri.Milliseconds();

	//Load all definitions
	GLSL_LoadDefinitions();

	tr.gl_genericShader                         = GLSL_GetShaderProgram("generic");
	tr.gl_lightMappingShader                    = GLSL_GetShaderProgram("lightMapping");
	tr.gl_vertexLightingShader_DBS_entity       = GLSL_GetShaderProgram("vertexLighting_DBS_entity");
	tr.gl_vertexLightingShader_DBS_world        = GLSL_GetShaderProgram("vertexLighting_DBS_world");
	tr.gl_forwardLightingShader_omniXYZ         = GLSL_GetShaderProgram("forwardLighting_omniXYZ");
	tr.gl_forwardLightingShader_projXYZ         = GLSL_GetShaderProgram("forwardLighting_projXYZ");
	tr.gl_forwardLightingShader_directionalSun  = GLSL_GetShaderProgram("forwardLighting_directionalSun");
	tr.gl_deferredLightingShader_omniXYZ        = GLSL_GetShaderProgram("deferredLighting_omniXYZ");
	tr.gl_deferredLightingShader_projXYZ        = GLSL_GetShaderProgram("deferredLighting_projXYZ");
	tr.gl_deferredLightingShader_directionalSun = GLSL_GetShaderProgram("deferredLighting_directionalSun");
	tr.gl_geometricFillShader                   = GLSL_GetShaderProgram("geometricFill");
	tr.gl_shadowFillShader                      = GLSL_GetShaderProgram("shadowFill");
	tr.gl_reflectionShader                      = GLSL_GetShaderProgram("reflection");
	tr.gl_skyboxShader                          = GLSL_GetShaderProgram("skybox");
	tr.gl_fogQuake3Shader                       = GLSL_GetShaderProgram("fogQuake3");
	tr.gl_fogGlobalShader                       = GLSL_GetShaderProgram("fogGlobal");
	tr.gl_heatHazeShader                        = GLSL_GetShaderProgram("heatHaze");
	tr.gl_screenShader                          = GLSL_GetShaderProgram("screen");
	tr.gl_portalShader                          = GLSL_GetShaderProgram("portal");
	tr.gl_toneMappingShader                     = GLSL_GetShaderProgram("toneMapping");
	tr.gl_contrastShader                        = GLSL_GetShaderProgram("contrast");
	tr.gl_cameraEffectsShader                   = GLSL_GetShaderProgram("cameraEffects");
	tr.gl_blurXShader                           = GLSL_GetShaderProgram("blurX");
	tr.gl_blurYShader                           = GLSL_GetShaderProgram("blurY");
	tr.gl_debugShadowMapShader                  = GLSL_GetShaderProgram("debugShadowMap");

	//Dushan
	tr.gl_liquidShader             = GLSL_GetShaderProgram("liquid");
	tr.gl_rotoscopeShader          = GLSL_GetShaderProgram("rotoscope");
	tr.gl_bloomShader              = GLSL_GetShaderProgram("bloom");
	tr.gl_refractionShader         = GLSL_GetShaderProgram("refraction");
	tr.gl_depthToColorShader       = GLSL_GetShaderProgram("depthToColor");
	tr.gl_volumetricFogShader      = GLSL_GetShaderProgram("volumetricFog");
	tr.gl_volumetricLightingShader = GLSL_GetShaderProgram("lightVolume_omni");
	tr.gl_dispersionShader         = GLSL_GetShaderProgram("dispersion");

	endTime = ri.Milliseconds();

	ri.Printf(PRINT_ALL, "loaded %i GLSL shaders (%i gen %i light %i etc) in %5.2f seconds\n",
	          numGenShaders + numLightShaders + numEtcShaders, numGenShaders, numLightShaders,
	          numEtcShaders, (endTime - startTime) / 1000.0);
}

void GLSL_ShutdownGPUShaders(void)
{
	int i;

	ri.Printf(PRINT_ALL, "------- GLSL_ShutdownGPUShaders -------\n");


	//FIXME: sort thrue this!
	/*
	qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD0);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD1);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_POSITION);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_POSITION2);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_NORMAL);
#ifdef USE_VERT_TANGENT_SPACE
	qglDisableVertexAttribArrayARB(ATTR_INDEX_TANGENT);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_BITANGENT);
#endif
	qglDisableVertexAttribArrayARB(ATTR_INDEX_NORMAL2);
#ifdef USE_VERT_TANGENT_SPACE
	qglDisableVertexAttribArrayARB(ATTR_INDEX_TANGENT2);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_BITANGENT2);
#endif
	qglDisableVertexAttribArrayARB(ATTR_INDEX_COLOR);
	qglDisableVertexAttribArrayARB(ATTR_INDEX_LIGHTDIRECTION);
	*/

	GLSL_BindNullProgram();

	//Clean up programInfo_t:s

	glState.currentProgram = 0;
	qglUseProgramObjectARB(0);
}


void GLSL_BindProgram(shaderProgram_t *program)
{
	if (!program)
	{
		GLSL_BindNullProgram();
		return;
	}

	if (r_logFile->integer)
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- GL_BindProgram( %s ) ---\n", program->name));
	}

	if (glState.currentProgram != program)
	{
		qglUseProgramObjectARB(program->program);
		glState.currentProgram = program;
		backEnd.pc.c_glslShaderBinds++;
	}
}


void GLSL_BindNullProgram(void)
{
	if (r_logFile->integer)
	{
		GLimp_LogComment("--- GL_BindNullProgram ---\n");
	}

	if (glState.currentProgram)
	{
		qglUseProgramObjectARB(0);
		glState.currentProgram = NULL;
	}
}
#endif // RENDERER2C

void GLSL_VertexAttribsState(uint32_t stateBits)
{
	uint32_t diff;

	if (glConfig2.vboVertexSkinningAvailable && tess.vboVertexSkinning)
	{
		stateBits |= (ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS);
	}

	GLSL_VertexAttribPointers(stateBits);

	diff = stateBits ^ glState.vertexAttribsState;
	if (!diff)
	{
		return;
	}

	if (diff & ATTR_POSITION)
	{
		if (stateBits & ATTR_POSITION)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_POSITION )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_POSITION);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_POSITION )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_POSITION);
		}
	}

	if (diff & ATTR_TEXCOORD)
	{
		if (stateBits & ATTR_TEXCOORD)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_TEXCOORD )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_TEXCOORD0);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_TEXCOORD )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_TEXCOORD0);
		}
	}

	if (diff & ATTR_LIGHTCOORD)
	{
		if (stateBits & ATTR_LIGHTCOORD)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_LIGHTCOORD )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_TEXCOORD1);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_LIGHTCOORD )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_TEXCOORD1);
		}
	}

	if (diff & ATTR_TANGENT)
	{
		if (stateBits & ATTR_TANGENT)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_TANGENT )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_TANGENT);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_TANGENT )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_TANGENT);
		}
	}

	if (diff & ATTR_BINORMAL)
	{
		if (stateBits & ATTR_BINORMAL)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_BINORMAL )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_BINORMAL);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_BINORMAL )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_BINORMAL);
		}
	}

	if (diff & ATTR_NORMAL)
	{
		if (stateBits & ATTR_NORMAL)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_NORMAL )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_NORMAL);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_NORMAL )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_NORMAL);
		}
	}

	if (diff & ATTR_COLOR)
	{
		if (stateBits & ATTR_COLOR)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_COLOR )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_COLOR);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_COLOR )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_COLOR);
		}
	}

	if (diff & ATTR_BONE_INDEXES)
	{
		if (stateBits & ATTR_BONE_INDEXES)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_BONE_INDEXES )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_BONE_INDEXES);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_BONE_INDEXES )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_BONE_INDEXES);
		}
	}

	if (diff & ATTR_BONE_WEIGHTS)
	{
		if (stateBits & ATTR_BONE_WEIGHTS)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_BONE_WEIGHTS )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_BONE_WEIGHTS);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_BONE_WEIGHTS )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_BONE_WEIGHTS);
		}
	}

	if (diff & ATTR_POSITION2)
	{
		if (stateBits & ATTR_POSITION2)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_POSITION2 )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_POSITION2);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_POSITION2 )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_POSITION2);
		}
	}

	if (diff & ATTR_TANGENT2)
	{
		if (stateBits & ATTR_TANGENT2)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_TANGENT2 )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_TANGENT2);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_TANGENT2 )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_TANGENT2);
		}
	}

	if (diff & ATTR_BINORMAL2)
	{
		if (stateBits & ATTR_BINORMAL2)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_BINORMAL2 )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_BINORMAL2);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_BINORMAL2 )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_BINORMAL2);
		}
	}

	if (diff & ATTR_NORMAL2)
	{
		if (stateBits & ATTR_NORMAL2)
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glEnableVertexAttribArray( ATTR_INDEX_NORMAL2 )\n");
			}
			glEnableVertexAttribArray(ATTR_INDEX_NORMAL2);
		}
		else
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glDisableVertexAttribArray( ATTR_INDEX_NORMAL2 )\n");
			}
			glDisableVertexAttribArray(ATTR_INDEX_NORMAL2);
		}
	}

	glState.vertexAttribsState = stateBits;
}

void GLSL_VertexAttribPointers(uint32_t attribBits)
{
	if (!glState.currentVBO)
	{
		ri.Error(ERR_FATAL, "GL_VertexAttribPointers: no VBO bound");
		return;
	}

	if (r_logFile->integer)
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- GL_VertexAttribPointers( %s ) ---\n", glState.currentVBO->name));
	}

	if (glConfig2.vboVertexSkinningAvailable && tess.vboVertexSkinning)
	{
		attribBits |= (ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS);
	}

	if ((attribBits & ATTR_POSITION))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_POSITION )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_POSITION, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsXYZ + (glState.vertexAttribsOldFrame * glState.currentVBO->sizeXYZ)));
		glState.vertexAttribPointersSet |= ATTR_POSITION;
	}

	if ((attribBits & ATTR_TEXCOORD))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_TEXCOORD )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_TEXCOORD0, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsTexCoords));
		glState.vertexAttribPointersSet |= ATTR_TEXCOORD;
	}

	if ((attribBits & ATTR_LIGHTCOORD))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_LIGHTCOORD )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_TEXCOORD1, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsLightCoords));
		glState.vertexAttribPointersSet |= ATTR_LIGHTCOORD;
	}

	if ((attribBits & ATTR_TANGENT))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_TANGENT )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_TANGENT, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(glState.currentVBO->ofsTangents + (glState.vertexAttribsOldFrame * glState.currentVBO->sizeTangents)));
		glState.vertexAttribPointersSet |= ATTR_TANGENT;
	}

	if ((attribBits & ATTR_BINORMAL))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_BINORMAL )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_BINORMAL, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(glState.currentVBO->ofsBinormals + (glState.vertexAttribsOldFrame * glState.currentVBO->sizeBinormals)));
		glState.vertexAttribPointersSet |= ATTR_BINORMAL;
	}

	if ((attribBits & ATTR_NORMAL))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_NORMAL )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_NORMAL, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(glState.currentVBO->ofsNormals + (glState.vertexAttribsOldFrame * glState.currentVBO->sizeNormals)));
		glState.vertexAttribPointersSet |= ATTR_NORMAL;
	}

	if ((attribBits & ATTR_COLOR))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_COLOR )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_COLOR, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsColors));
		glState.vertexAttribPointersSet |= ATTR_COLOR;
	}

	if ((attribBits & ATTR_BONE_INDEXES))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_BONE_INDEXES )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_BONE_INDEXES, 4, GL_INT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsBoneIndexes));
		glState.vertexAttribPointersSet |= ATTR_BONE_INDEXES;
	}

	if ((attribBits & ATTR_BONE_WEIGHTS))
	{
		if (r_logFile->integer)
		{
			GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_BONE_WEIGHTS )\n");
		}

		glVertexAttribPointer(ATTR_INDEX_BONE_WEIGHTS, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsBoneWeights));
		glState.vertexAttribPointersSet |= ATTR_BONE_WEIGHTS;
	}

	if (glState.vertexAttribsInterpolation > 0)
	{
		if ((attribBits & ATTR_POSITION2))
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_POSITION2 )\n");
			}

			glVertexAttribPointer(ATTR_INDEX_POSITION2, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(glState.currentVBO->ofsXYZ + (glState.vertexAttribsNewFrame * glState.currentVBO->sizeXYZ)));
			glState.vertexAttribPointersSet |= ATTR_POSITION2;
		}

		if ((attribBits & ATTR_TANGENT2))
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_TANGENT2 )\n");
			}

			glVertexAttribPointer(ATTR_INDEX_TANGENT2, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(glState.currentVBO->ofsTangents + (glState.vertexAttribsNewFrame * glState.currentVBO->sizeTangents)));
			glState.vertexAttribPointersSet |= ATTR_TANGENT2;
		}

		if ((attribBits & ATTR_BINORMAL2))
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_BINORMAL2 )\n");
			}

			glVertexAttribPointer(ATTR_INDEX_BINORMAL2, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(glState.currentVBO->ofsBinormals + (glState.vertexAttribsNewFrame * glState.currentVBO->sizeBinormals)));
			glState.vertexAttribPointersSet |= ATTR_BINORMAL2;
		}

		if ((attribBits & ATTR_NORMAL2))
		{
			if (r_logFile->integer)
			{
				GLimp_LogComment("glVertexAttribPointer( ATTR_INDEX_NORMAL2 )\n");
			}

			glVertexAttribPointer(ATTR_INDEX_NORMAL2, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(glState.currentVBO->ofsNormals + (glState.vertexAttribsNewFrame * glState.currentVBO->sizeNormals)));
			glState.vertexAttribPointersSet |= ATTR_NORMAL2;
		}
	}
}