/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_RENDER_H
#define GAME_CLIENT_RENDER_H

#include <base/color.h>
#include <base/vmath.h>

#include <game/client/skin.h>
#include <game/client/ui_rect.h>

class CAnimState;
class CSpeedupTile;
class CSwitchTile;
class CTeleTile;
class CTile;
class CTuneTile;
namespace client_data7 {
struct CDataSprite;
}
struct CDataSprite;
struct CEnvPoint;
struct CEnvPointBezier;
struct CEnvPointBezier_upstream;
struct CMapItemGroup;
struct CQuad;

class CTeeRenderInfo
{
public:
	CTeeRenderInfo()
	{
		Reset();
	}

	void Reset()
	{
		m_OriginalRenderSkin.Reset();
		m_ColorableRenderSkin.Reset();
		m_SkinMetrics.Reset();
		m_CustomColoredSkin = false;
		m_BloodColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		m_ColorBody = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		m_ColorFeet = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		m_Size = 1.0f;
		m_GotAirJump = true;
		m_TeeRenderFlags = 0;
		m_FeetFlipped = false;
	}

	CSkin::SSkinTextures m_OriginalRenderSkin;
	CSkin::SSkinTextures m_ColorableRenderSkin;

	CSkin::SSkinMetrics m_SkinMetrics;

	bool m_CustomColoredSkin;
	ColorRGBA m_BloodColor;

	ColorRGBA m_ColorBody;
	ColorRGBA m_ColorFeet;
	float m_Size;
	bool m_GotAirJump;
	int m_TeeRenderFlags;
	bool m_FeetFlipped;

	bool Valid() const
	{
		return m_CustomColoredSkin ? m_ColorableRenderSkin.m_Body.IsValid() : m_OriginalRenderSkin.m_Body.IsValid();
	}
};

// Tee Render Flags
enum
{
	TEE_EFFECT_FROZEN = 1,
	TEE_NO_WEAPON = 2,
};

// sprite renderings
enum
{
	SPRITE_FLAG_FLIP_Y = 1,
	SPRITE_FLAG_FLIP_X = 2,

	LAYERRENDERFLAG_OPAQUE = 1,
	LAYERRENDERFLAG_TRANSPARENT = 2,

	TILERENDERFLAG_EXTEND = 4,
};

class IEnvelopePointAccess
{
public:
	virtual ~IEnvelopePointAccess() = default;
	virtual int NumPoints() const = 0;
	virtual const CEnvPoint *GetPoint(int Index) const = 0;
	virtual const CEnvPointBezier *GetBezier(int Index) const = 0;
};

class CMapBasedEnvelopePointAccess : public IEnvelopePointAccess
{
	int m_StartPoint;
	int m_NumPoints;
	int m_NumPointsMax;
	CEnvPoint *m_pPoints;
	CEnvPointBezier *m_pPointsBezier;
	CEnvPointBezier_upstream *m_pPointsBezierUpstream;

public:
	CMapBasedEnvelopePointAccess(class CDataFileReader *pReader);
	CMapBasedEnvelopePointAccess(class IMap *pMap);
	void SetPointsRange(int StartPoint, int NumPoints);
	int StartPoint() const;
	int NumPoints() const override;
	int NumPointsMax() const;
	const CEnvPoint *GetPoint(int Index) const override;
	const CEnvPointBezier *GetBezier(int Index) const override;
};

typedef void (*ENVELOPE_EVAL)(int TimeOffsetMillis, int Env, ColorRGBA &Channels, void *pUser);

class CRenderTools
{
	class IGraphics *m_pGraphics;
	class ITextRender *m_pTextRender;

	int m_TeeQuadContainerIndex;

	static void GetRenderTeeBodyScale(float BaseSize, float &BodyScale);
	static void GetRenderTeeFeetScale(float BaseSize, float &FeetScaleWidth, float &FeetScaleHeight);

public:
	class IGraphics *Graphics() const { return m_pGraphics; }
	class ITextRender *TextRender() const { return m_pTextRender; }

	void Init(class IGraphics *pGraphics, class ITextRender *pTextRender);

	void SelectSprite(CDataSprite *pSprite, int Flags = 0, int sx = 0, int sy = 0) const;
	void SelectSprite(int Id, int Flags = 0, int sx = 0, int sy = 0) const;

	void GetSpriteScale(const CDataSprite *pSprite, float &ScaleX, float &ScaleY) const;
	void GetSpriteScale(int Id, float &ScaleX, float &ScaleY) const;
	void GetSpriteScaleImpl(int Width, int Height, float &ScaleX, float &ScaleY) const;

	void DrawSprite(float x, float y, float Size) const;
	void DrawSprite(float x, float y, float ScaledWidth, float ScaledHeight) const;
	void RenderCursor(vec2 Center, float Size) const;
	void RenderIcon(int ImageId, int SpriteId, const CUIRect *pRect, const ColorRGBA *pColor = nullptr) const;
	int QuadContainerAddSprite(int QuadContainerIndex, float x, float y, float Size) const;
	int QuadContainerAddSprite(int QuadContainerIndex, float Size) const;
	int QuadContainerAddSprite(int QuadContainerIndex, float Width, float Height) const;
	int QuadContainerAddSprite(int QuadContainerIndex, float X, float Y, float Width, float Height) const;

	// larger rendering methods
	static void GetRenderTeeBodySize(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, vec2 &BodyOffset, float &Width, float &Height);
	static void GetRenderTeeFeetSize(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, vec2 &FeetOffset, float &Width, float &Height);
	static void GetRenderTeeAnimScaleAndBaseSize(const CTeeRenderInfo *pInfo, float &AnimScale, float &BaseSize);

	// returns the offset to use, to render the tee with @see RenderTee exactly in the mid
	static void GetRenderTeeOffsetToRenderedTee(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, vec2 &TeeOffsetToMid);
	// object render methods
	void RenderTee(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha = 1.0f) const;

	// map render methods (render_map.cpp)
	static void RenderEvalEnvelope(const IEnvelopePointAccess *pPoints, int Channels, std::chrono::nanoseconds TimeNanos, ColorRGBA &Result);
	void RenderQuads(CQuad *pQuads, int NumQuads, int Flags, ENVELOPE_EVAL pfnEval, void *pUser) const;
	void ForceRenderQuads(CQuad *pQuads, int NumQuads, int Flags, ENVELOPE_EVAL pfnEval, void *pUser, float Alpha = 1.0f) const;
	void RenderTilemap(CTile *pTiles, int w, int h, float Scale, ColorRGBA Color, int RenderFlags, ENVELOPE_EVAL pfnEval, void *pUser, int ColorEnv, int ColorEnvOffset) const;

	// render a rectangle made of IndexIn tiles, over a background made of IndexOut tiles
	// the rectangle include all tiles in [RectX, RectX+RectW-1] x [RectY, RectY+RectH-1]
	void RenderTileRectangle(int RectX, int RectY, int RectW, int RectH, unsigned char IndexIn, unsigned char IndexOut, float Scale, ColorRGBA Color, int RenderFlags, ENVELOPE_EVAL pfnEval, void *pUser, int ColorEnv, int ColorEnvOffset) const;

	// helpers
	void CalcScreenParams(float Aspect, float Zoom, float *pWidth, float *pHeight);
	void MapScreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
		float ParallaxZoom, float OffsetX, float OffsetY, float Aspect, float Zoom, float *pPoints);
	void MapScreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup, float Zoom);
	void MapScreenToInterface(float CenterX, float CenterY);

	// DDRace

	void RenderTeleOverlay(CTeleTile *pTele, int w, int h, float Scale, float Alpha = 1.0f) const;
	void RenderSpeedupOverlay(CSpeedupTile *pSpeedup, int w, int h, float Scale, float Alpha = 1.0f) const;
	void RenderSwitchOverlay(CSwitchTile *pSwitch, int w, int h, float Scale, float Alpha = 1.0f) const;
	void RenderTuneOverlay(CTuneTile *pTune, int w, int h, float Scale, float Alpha = 1.0f) const;
	void RenderTelemap(CTeleTile *pTele, int w, int h, float Scale, ColorRGBA Color, int RenderFlags) const;
	void RenderSpeedupmap(CSpeedupTile *pSpeedup, int w, int h, float Scale, ColorRGBA Color, int RenderFlags) const;
	void RenderSwitchmap(CSwitchTile *pSwitch, int w, int h, float Scale, ColorRGBA Color, int RenderFlags) const;
	void RenderTunemap(CTuneTile *pTune, int w, int h, float Scale, ColorRGBA Color, int RenderFlags) const;
};

#endif
