// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OpenGLDrvPrivate.h"

#include "MacOpenGLQuery.h"

/*------------------------------------------------------------------------------
 OpenGL static variables.
 ------------------------------------------------------------------------------*/

bool GIsEmulatingTimestamp = false;
static const uint32 GMacQueryNameCacheSize = 32 * OPENGL_NAME_CACHE_SIZE;

char const* const GMacTimerVertShader4 =	"#version 150\n"
											"const int VertexCount = 4;\n"
											"const vec2 Position[VertexCount] = vec2[](	vec2(-1.0,-1.0),\n"
											"											vec2( 1.0,-1.0),\n"
											"											vec2(-1.0, 1.0),\n"
											"											vec2( 1.0, 1.0));\n"
											"void main()\n"
											"{\n"
											"	gl_Position = vec4(Position[gl_VertexID], 0.0, 1.0);\n"
											"}\n";

char const* const GMacTimerFragShader =	"#version 150\n"
										"out vec4 Color;\n"
										"void main()\n"
										"{\n"
										"	Color = vec4(0.0, 0.0, 0.0, 1.0);\n"
										"}\n";

GLuint FMacOpenGLQueryEmu::DummyDrawProgram = 0;

/*------------------------------------------------------------------------------
 OpenGL query emulation.
 ------------------------------------------------------------------------------*/

FMacOpenGLTimer::FMacOpenGLTimer(FPlatformOpenGLContext* InContext, FMacOpenGLQueryEmu* InEmu)
: Context(InContext)
, Emu(InEmu)
, Name(0)
, Result(0)
, Accumulated(0)
, Available(0)
, Cached(false)
, Running(false)
{
	check(Context);
	check(Emu);
	
	if(!Emu->FreeQueries.FindOrAdd(GL_TIME_ELAPSED).Num())
	{
		if(IsRHIDeviceNVIDIA())
		{
			glFinish();
		}
		
		Emu->FreeQueries.FindOrAdd(GL_TIME_ELAPSED).AddUninitialized(GMacQueryNameCacheSize);
		glGenQueries(GMacQueryNameCacheSize, Emu->FreeQueries.FindOrAdd(GL_TIME_ELAPSED).GetData());
	}
	Name = Emu->FreeQueries.FindOrAdd(GL_TIME_ELAPSED).Pop();
}

FMacOpenGLTimer::~FMacOpenGLTimer()
{
	End();
	Next.Reset();
	if (Name)
	{
		Emu->FreeQueries.FindOrAdd(GL_TIME_ELAPSED).Push(Name);
		Name = 0;
	}
}

void FMacOpenGLTimer::Begin(void)
{
	if(!Running)
	{
		glBeginQuery(GL_TIME_ELAPSED, Name);
		Running = true;
	}
}

void FMacOpenGLTimer::End(void)
{
	if(Running)
	{
		Running = false;
		glEndQuery(GL_TIME_ELAPSED);
	}
}

uint64 FMacOpenGLTimer::GetResult(void)
{
	check(!Running);
	CacheResult();
	return Result;
}

uint64 FMacOpenGLTimer::GetAccumulatedResult(void)
{
	check(!Running);
	CacheResult();
	return Accumulated;
}

int32 FMacOpenGLTimer::GetResultAvailable(void)
{
	check(!Running);
	if(!Available)
	{
		glGetQueryObjectuiv(Name, GL_QUERY_RESULT_AVAILABLE, &Available);
	}
	return Available;
}

void FMacOpenGLTimer::CacheResult(void)
{
	check(!Running);
	if(!Cached)
	{
		glGetQueryObjectui64v(Name, GL_QUERY_RESULT, &Result);
		
		if(Previous.IsValid())
		{
			Accumulated += Result;
			Accumulated += Previous->GetAccumulatedResult();
			
			Previous->Next.Reset();
			Previous.Reset();
		}
		
		Available = true;
		Cached = true;
	}
}

FMacOpenGLQuery::FMacOpenGLQuery(FPlatformOpenGLContext* InContext, FMacOpenGLQueryEmu* InEmu)
	: Name(0)
	, Target(0)
	, Context(InContext)
	, Emu(InEmu)
	, Result(0)
	, Available(0)
	, Running(0)
	, Cached(false)
{
	check(Context);
	check(Emu);
}

FMacOpenGLQuery::~FMacOpenGLQuery()
{
	if(Running)
	{
		End();
	}
	if(Name)
	{
		switch(Target)
		{
			case GL_TIMESTAMP:
			case GL_TIME_ELAPSED:
			{
				Emu->FreeQueries.FindOrAdd(GL_TIME_ELAPSED).Push(Name);
				break;
			}
			default:
			{
				Emu->FreeQueries.FindOrAdd(Target).Push(Name);
				break;
			}
		}
		Name = 0;
	}
}

void FMacOpenGLQuery::Begin(GLenum InTarget)
{
	if(Target == 0 || Target == InTarget)
	{
		Target = InTarget;
		Result = 0;
		Running = true;
		Available = false;
		Cached = false;
		Start.Reset();
		Finish.Reset();
		switch(Target)
		{
			case GL_TIMESTAMP:
			{
				// There can be a lot of timestamps, clear emulated queries out to avoid problems.
				for( auto It : Emu->Queries )
				{
					if(It.IsValid() && It->Running == false && (It->Target == GL_TIMESTAMP || It->Target == GL_TIME_ELAPSED))
					{
						if(It->GetResultAvailable())
						{
							It->GetResult();
						}
						else
						{
							break;
						}
					}
				}
				
				check(Emu->LastTimer.IsValid());
				Emu->LastTimer->End();
				Start = Emu->LastTimer;
				
				TSharedPtr<FMacOpenGLTimer, ESPMode::Fast> Current(new FMacOpenGLTimer(Context, Emu));
				Current->Previous = Start;
				Start->Next = Current;
				Current->Begin();
				Emu->LastTimer = Current;
				
				break;
			}
			case GL_TIME_ELAPSED:
			{
				check(Emu->LastTimer.IsValid());
				Emu->LastTimer->End();
				
				Start = TSharedPtr<FMacOpenGLTimer, ESPMode::Fast>(new FMacOpenGLTimer(Context, Emu));
				Start->Previous = Emu->LastTimer;
				Emu->LastTimer->Next = Start;
				Start->Begin();
				
				Emu->LastTimer = Start;
				break;
			}
			default:
			{
				if(!Name)
				{
					if(!Emu->FreeQueries.FindOrAdd(Target).Num())
					{
						if(IsRHIDeviceNVIDIA())
						{
							glFinish();
						}
						
						Emu->FreeQueries.FindOrAdd(Target).AddUninitialized(GMacQueryNameCacheSize);
						glGenQueries(GMacQueryNameCacheSize, Emu->FreeQueries.FindOrAdd(Target).GetData());
					}
					Name = Emu->FreeQueries.FindOrAdd(Target).Pop();
				}
				glBeginQuery(Target, Name);
				break;
			}
		}
	}
}

void FMacOpenGLQuery::End()
{
	Running = false;
	if(Target == GL_TIME_ELAPSED)
	{
		Finish = Emu->LastTimer;
		TSharedPtr<FMacOpenGLTimer, ESPMode::Fast> Current(new FMacOpenGLTimer(Context, Emu));
		Current->Previous = Finish;
		Finish->Next = Current;
		Finish->End();
		Current->Begin();
		Emu->LastTimer = Current;
		
		if (Start == Finish)
		{
			Start = Start->Previous;
		}
	}
	else if(Target != GL_TIMESTAMP)
	{
		glEndQuery(Target);
	}
}

uint64 FMacOpenGLQuery::GetResult(void)
{
	if(!Cached)
	{
		switch(Target)
		{
			case GL_TIMESTAMP:
				Result = Start->GetAccumulatedResult();
				Start.Reset();
				Finish.Reset();
				break;
			case GL_TIME_ELAPSED:
				Result = Finish->GetAccumulatedResult() - Start->GetAccumulatedResult();
				Start.Reset();
				Finish.Reset();
				break;
			default:
				glGetQueryObjectui64v(Name, GL_QUERY_RESULT, &Result);
				break;
		}
		Available = true;
		Cached = true;
	}
	return Result;
}

int32 FMacOpenGLQuery::GetResultAvailable(void)
{
	if(!Available)
	{
		switch(Target)
		{
			case GL_TIMESTAMP:
				Available = Start->GetResultAvailable();
				break;
			case GL_TIME_ELAPSED:
				Available = Finish->GetResultAvailable();
				break;
			default:
				glGetQueryObjectuiv(Name, GL_QUERY_RESULT_AVAILABLE, &Available);
				break;
		}
	}
	return Available;
}

FMacOpenGLQueryEmu::FMacOpenGLQueryEmu(FPlatformOpenGLContext* InContext)
: PlatformContext(InContext)
, bOwnsDummyDrawProgram( InContext == nullptr )
{
	check(PlatformContext);
	
	// Only use the timestamp emulation in a non-shipping build - end-users shouldn't care about this profiling feature.
#if (!UE_BUILD_SHIPPING)
	GIsEmulatingTimestamp = !FParse::Param(FCommandLine::Get(), TEXT("DisableMacGPUTimestamp"));
#endif
	
	if ( GIsEmulatingTimestamp )
	{
		FreeQueries.FindOrAdd(GL_TIME_ELAPSED).AddUninitialized(GMacQueryNameCacheSize);
		glGenQueries(GMacQueryNameCacheSize, FreeQueries.FindOrAdd(GL_TIME_ELAPSED).GetData());
		
		FreeQueries.FindOrAdd(GL_SAMPLES_PASSED).AddUninitialized(GMacQueryNameCacheSize);
		glGenQueries(GMacQueryNameCacheSize, FreeQueries.FindOrAdd(GL_SAMPLES_PASSED).GetData());
		
		GLuint Query = FreeQueries.FindOrAdd(GL_TIME_ELAPSED).Pop();
		glBeginQuery(GL_TIME_ELAPSED, Query);
		{
			GLuint ShaderProgram = GetDummyDrawProgram();
			glUseProgram(ShaderProgram);
			
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, 1);
			
			glUseProgram(0);
		}
		glEndQuery(GL_TIME_ELAPSED);
		glFlushRenderAPPLE();
		FreeQueries.FindOrAdd(GL_TIME_ELAPSED).Push(Query);
		
		LastTimer = TSharedPtr<FMacOpenGLTimer, ESPMode::Fast>(new FMacOpenGLTimer(InContext, this));
		LastTimer->Begin();
	}
}

FMacOpenGLQueryEmu::~FMacOpenGLQueryEmu()
{
	if ( GIsEmulatingTimestamp )
	{
		if( LastTimer.IsValid() )
		{
			LastTimer->End();
			LastTimer = nullptr;
		}
			
		Queries.Empty();
		RunningQueries.Empty();
		
		if(IsRHIDeviceNVIDIA())
		{
			glFinish();
		}
		
		for (auto Pair : FreeQueries)
		{
			glDeleteQueries(Pair.Value.Num(), Pair.Value.GetData());
		}
		FreeQueries.Empty();
		
		if ( bOwnsDummyDrawProgram )
		{
			glDeleteProgram(DummyDrawProgram);
			DummyDrawProgram = 0;
		}
	}
}
	
bool FMacOpenGLQueryEmu::IsQuery(GLuint const QueryID) const
{
	return QueryID && Queries.Num() >= QueryID && Queries[QueryID-1].IsValid();
}

TSharedPtr<FMacOpenGLQuery, ESPMode::Fast> FMacOpenGLQueryEmu::GetQuery(GLuint const QueryID) const
{
	TSharedPtr<FMacOpenGLQuery, ESPMode::Fast> Query;
	if ( IsQuery(QueryID) )
	{
		Query = Queries[QueryID-1];
	}
	return Query;
}

GLuint FMacOpenGLQueryEmu::CreateQuery(void)
{
	GLuint Result = 0;
	if ( Queries.Num() < UINT_MAX )
	{
		TSharedPtr<FMacOpenGLQuery, ESPMode::Fast> Query(new FMacOpenGLQuery(PlatformContext, this));
		for(int32 Index = 0; Index < Queries.Num(); ++Index)
		{
			if(!Queries[Index].IsValid())
			{
				Queries[Index] = Query;
				Result = Index + 1;
				break;
			}
		}
		
		if(!Result)
		{
			Queries.Add(Query);
			Result = Queries.Num();
		}
	}
	return Result;
}

void FMacOpenGLQueryEmu::DeleteQuery(GLuint const QueryID)
{
	if ( IsQuery(QueryID) )
	{
		Queries[QueryID - 1].Reset();
	}
}

void FMacOpenGLQueryEmu::BeginQuery(GLenum const QueryType, GLuint const QueryID)
{
	TSharedPtr<FMacOpenGLQuery, ESPMode::Fast> Query = GetQuery( QueryID );
	if ( Query.IsValid() )
	{
		Query->Begin(QueryType);
		RunningQueries.Add(QueryType, Query);
	}
}

void FMacOpenGLQueryEmu::EndQuery(GLenum const QueryType)
{
	TSharedPtr<FMacOpenGLQuery> Query = RunningQueries.FindOrAdd(QueryType);
	if ( Query.IsValid() )
	{
		Query->End();
		RunningQueries[QueryType].Reset();
	}
}

GLuint FMacOpenGLQueryEmu::GetDummyDrawProgram()
{
	if ( !DummyDrawProgram )
	{
		GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(VertexShader, 1, &GMacTimerVertShader4, NULL);
		glCompileShader(VertexShader);
		GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(FragmentShader, 1, &GMacTimerFragShader, NULL);
		glCompileShader(FragmentShader);
		GLuint ShaderProgram = glCreateProgram();
		glAttachShader(ShaderProgram, VertexShader);
		glAttachShader(ShaderProgram, FragmentShader);
		glBindFragDataLocation(ShaderProgram, 0, "Color");
		
		glLinkProgram(ShaderProgram);
		glValidateProgram(ShaderProgram);
		
		glDeleteShader(VertexShader);
		glDeleteShader(FragmentShader);
		
		DummyDrawProgram = ShaderProgram;
	}
	
	return DummyDrawProgram;
}
