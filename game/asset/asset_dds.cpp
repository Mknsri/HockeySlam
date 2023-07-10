#include "asset_dds.h"

#define GL_BGR_EXT 0x80E0
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

namespace Asset {
// pixel format flags
const uint32_t DDSF_FOURCC = 0x00000004;
const uint32_t DDSF_RGB = 0x00000040;

// Caps2 flags
const uint32_t DDSF_CUBEMAP = 0x00000200;
const uint32_t DDSF_VOLUME = 0x00200000;

// compressed texture types
const uint32_t FOURCC_DXT1 = 0x31545844; //(MAKEFOURCC('D','X','T','1'))
const uint32_t FOURCC_DXT3 = 0x33545844; //(MAKEFOURCC('D','X','T','3'))
const uint32_t FOURCC_DXT5 = 0x35545844; //(MAKEFOURCC('D','X','T','5'))

struct DDS_PIXELFORMAT
{
  uint32_t Size;
  uint32_t Flags;
  uint32_t FourCC;
  uint32_t RGBBitCount;
  uint32_t RBitMask;
  uint32_t GBitMask;
  uint32_t BBitMask;
  uint32_t ABitMask;
};

struct DDS_HEADER
{
  uint32_t Size;
  uint32_t Flags;
  uint32_t Height;
  uint32_t Width;
  uint32_t PitchOrLinearSize;
  uint32_t Depth;
  uint32_t MipMapCount;
  uint32_t Reserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t Caps1;
  uint32_t Caps2;
  uint32_t Reserved2[3];
};

struct DXTColBlock
{
  uint16_t col0;
  uint16_t col1;

  uint8_t row[4];
};

struct DXT3AlphaBlock
{
  uint16_t row[4];
};

struct DXT5AlphaBlock
{
  uint8_t alpha0;
  uint8_t alpha1;

  uint8_t row[6];
};

///////////////////////////////////////////////////////////////////////////////
// clamps input size to [1-size]
inline uint32_t clamp_size(uint32_t size)
{
  if (size <= 0) {
    size = 1;
  }

  return size;
}

///////////////////////////////////////////////////////////////////////////////
// CDDSImage private functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// calculates size of DXTC texture in bytes
inline uint32_t size_dxtc(uint32_t width,
                          uint32_t height,
                          const DDS_Image& image)
{
  return ((width + 3) / 4) * ((height + 3) / 4) *
         (image.Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
}

///////////////////////////////////////////////////////////////////////////////
// calculates size of uncompressed RGB texture in bytes
inline uint32_t size_rgb(uint32_t width,
                         uint32_t height,
                         const DDS_Image& image)
{
  return width * height * image.Components;
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT1 color block
void flip_blocks_dxtc1(DXTColBlock* line, uint32_t numBlocks)
{
  DXTColBlock* curblock = line;

  for (uint32_t i = 0; i < numBlocks; i++) {
    std::swap(curblock->row[0], curblock->row[3]);
    std::swap(curblock->row[1], curblock->row[2]);

    curblock++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT3 color block
void flip_blocks_dxtc3(DXTColBlock* line, uint32_t numBlocks)
{
  DXTColBlock* curblock = line;
  DXT3AlphaBlock* alphablock;

  for (uint32_t i = 0; i < numBlocks; i++) {
    alphablock = (DXT3AlphaBlock*)curblock;

    std::swap(alphablock->row[0], alphablock->row[3]);
    std::swap(alphablock->row[1], alphablock->row[2]);

    curblock++;

    std::swap(curblock->row[0], curblock->row[3]);
    std::swap(curblock->row[1], curblock->row[2]);

    curblock++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 alpha block
void flip_dxt5_alpha(DXT5AlphaBlock* block)
{
  uint8_t gBits[4][4];

  const uint32_t mask = 0x00000007; // bits = 00 00 01 11
  uint32_t bits = 0;
  memcpy(&bits, &block->row[0], sizeof(uint8_t) * 3);

  gBits[0][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[0][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[0][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[0][3] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[1][3] = (uint8_t)(bits & mask);

  bits = 0;
  memcpy(&bits, &block->row[3], sizeof(uint8_t) * 3);

  gBits[2][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[2][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[2][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[2][3] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][0] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][1] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][2] = (uint8_t)(bits & mask);
  bits >>= 3;
  gBits[3][3] = (uint8_t)(bits & mask);

  uint32_t* pBits = ((uint32_t*)&(block->row[0]));

  *pBits = *pBits | (gBits[3][0] << 0);
  *pBits = *pBits | (gBits[3][1] << 3);
  *pBits = *pBits | (gBits[3][2] << 6);
  *pBits = *pBits | (gBits[3][3] << 9);

  *pBits = *pBits | (gBits[2][0] << 12);
  *pBits = *pBits | (gBits[2][1] << 15);
  *pBits = *pBits | (gBits[2][2] << 18);
  *pBits = *pBits | (gBits[2][3] << 21);

  pBits = ((uint32_t*)&(block->row[3]));

#ifdef MACOS
  *pBits &= 0x000000ff;
#else
  *pBits &= 0xff000000;
#endif

  *pBits = *pBits | (gBits[1][0] << 0);
  *pBits = *pBits | (gBits[1][1] << 3);
  *pBits = *pBits | (gBits[1][2] << 6);
  *pBits = *pBits | (gBits[1][3] << 9);

  *pBits = *pBits | (gBits[0][0] << 12);
  *pBits = *pBits | (gBits[0][1] << 15);
  *pBits = *pBits | (gBits[0][2] << 18);
  *pBits = *pBits | (gBits[0][3] << 21);
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 color block
void flip_blocks_dxtc5(DXTColBlock* line, uint32_t numBlocks)
{
  DXTColBlock* curblock = line;
  DXT5AlphaBlock* alphablock;

  for (uint32_t i = 0; i < numBlocks; i++) {
    alphablock = (DXT5AlphaBlock*)curblock;

    flip_dxt5_alpha(alphablock);

    curblock++;

    std::swap(curblock->row[0], curblock->row[3]);
    std::swap(curblock->row[1], curblock->row[2]);

    curblock++;
  }
}

bool is_compressed(const DDS_Image& image)
{
  return (image.Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
         (image.Format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) ||
         (image.Format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
}

///////////////////////////////////////////////////////////////////////////////
// flip image around X axis
void flip(DDS_Surface* surface, const DDS_Image& image)
{
  uint32_t linesize;
  uint32_t offset;

  if (!is_compressed(image)) {
    HOKI_ASSERT(surface->Depth > 0);

    uint32_t imagesize = surface->Size / surface->Depth;
    linesize = imagesize / surface->Height;

    uint8_t* tmp = new uint8_t[linesize];

    for (uint32_t n = 0; n < surface->Depth; n++) {
      offset = imagesize * n;
      uint8_t* top = (uint8_t*)surface->Pixels + offset;
      uint8_t* bottom = top + (imagesize - linesize);

      for (uint32_t i = 0; i < (surface->Height >> 1); i++) {
        // std::swap
        memcpy(tmp, bottom, linesize);
        memcpy(bottom, top, linesize);
        memcpy(top, tmp, linesize);

        top += linesize;
        bottom -= linesize;
      }
    }

    delete[] tmp;
  } else {
    void (*flipblocks)(DXTColBlock*, uint32_t);
    uint32_t xblocks = surface->Width / 4;
    uint32_t yblocks = surface->Height / 4;
    uint32_t blocksize;

    switch (image.Format) {
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        blocksize = 8;
        flipblocks = flip_blocks_dxtc1;
        break;
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        blocksize = 16;
        flipblocks = flip_blocks_dxtc3;
        break;
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        blocksize = 16;
        flipblocks = flip_blocks_dxtc5;
        break;
      default:
        return;
    }

    linesize = xblocks * blocksize;

    DXTColBlock* top;
    DXTColBlock* bottom;

    uint8_t* tmp = new uint8_t[linesize];

    for (uint32_t j = 0; j < (yblocks >> 1); j++) {
      top = (DXTColBlock*)((uint8_t*)surface->Pixels + j * linesize);
      bottom = (DXTColBlock*)((uint8_t*)surface->Pixels +
                              (((yblocks - j) - 1) * linesize));

      flipblocks(top, xblocks);
      flipblocks(bottom, xblocks);

      // std::swap
      memcpy(tmp, bottom, linesize);
      memcpy(bottom, top, linesize);
      memcpy(top, tmp, linesize);
    }

    delete[] tmp;
  }
}

DDS_Image createDDS(const uint8_t* memory, bool flipImage)
{
  DDS_Image resultImage = {};

  // read in file marker, make sure its a DDS file
  uint8_t* cursor = (uint8_t*)memory;
  char* filecode = (char*)cursor;
  cursor += 4;

  if (strncmp(filecode, "DDS ", 4) != 0) {
    HOKI_ASSERT(false);
  }

  // read in DDS header
  DDS_HEADER* ddsh = (DDS_HEADER*)cursor;
  cursor += sizeof(DDS_HEADER);

  // default to flat texture type (1D, 2D, or rectangle)
  resultImage.Type = DDS_TEXTURE_FLAT;

  // check if image is a cubemap
  if (ddsh->Caps2 & DDSF_CUBEMAP)
    resultImage.Type = DDS_TEXTURE_CUBEMAP;

  // check if image is a volume texture
  if ((ddsh->Caps2 & DDSF_VOLUME) && (ddsh->Depth > 0))
    resultImage.Type = DDS_TEXTURE_3D;

  // figure out what the image format is
  if (ddsh->ddspf.Flags & DDSF_FOURCC) {
    switch (ddsh->ddspf.FourCC) {
      case FOURCC_DXT1:
        resultImage.Format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        resultImage.Components = 3;
        break;
      case FOURCC_DXT3:
        resultImage.Format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        resultImage.Components = 4;
        break;
      case FOURCC_DXT5:
        resultImage.Format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        resultImage.Components = 4;
        break;
      default:
        HOKI_ASSERT(false);
    }
  } else if (ddsh->ddspf.RGBBitCount == 32 &&
             ddsh->ddspf.RBitMask == 0x00FF0000 &&
             ddsh->ddspf.GBitMask == 0x0000FF00 &&
             ddsh->ddspf.BBitMask == 0x000000FF &&
             ddsh->ddspf.ABitMask == 0xFF000000) {
    resultImage.Format = 0x80E1; // GL_BGRA_EXT;
    resultImage.Components = 4;
  } else if (ddsh->ddspf.RGBBitCount == 32 &&
             ddsh->ddspf.RBitMask == 0x000000FF &&
             ddsh->ddspf.GBitMask == 0x0000FF00 &&
             ddsh->ddspf.BBitMask == 0x00FF0000 &&
             ddsh->ddspf.ABitMask == 0xFF000000) {
    resultImage.Format = 0x1908; // GL_RGBA;
    resultImage.Components = 4;
  } else if (ddsh->ddspf.RGBBitCount == 24 &&
             ddsh->ddspf.RBitMask == 0x000000FF &&
             ddsh->ddspf.GBitMask == 0x0000FF00 &&
             ddsh->ddspf.BBitMask == 0x00FF0000) {
    resultImage.Format = 0x1907; // GL_RGB;
    resultImage.Components = 3;
  } else if (ddsh->ddspf.RGBBitCount == 24 &&
             ddsh->ddspf.RBitMask == 0x00FF0000 &&
             ddsh->ddspf.GBitMask == 0x0000FF00 &&
             ddsh->ddspf.BBitMask == 0x000000FF) {
    resultImage.Format = 0x80E0; // GL_BGR_EXT;
    resultImage.Components = 3;
  } else if (ddsh->ddspf.RGBBitCount == 8) {
    resultImage.Format = 0x1909; // GL_LUMINANCE;
    resultImage.Components = 1;
  } else {
    HOKI_ASSERT(false);
  }

  // store primary surface width/height/depth
  uint32_t width, height, depth;
  width = ddsh->Width;
  height = ddsh->Height;
  depth = clamp_size(ddsh->Depth); // set to 1 if 0

  // use correct size calculation function depending on whether image is
  // compressed
  uint32_t (*sizefunc)(uint32_t, uint32_t, const DDS_Image&);
  sizefunc = (is_compressed(resultImage) ? &size_dxtc : &size_rgb);

  // load all surfaces for the image (6 surfaces for cubemaps)
  for (uint32_t n = 0;
       n < (uint32_t)(resultImage.Type == DDS_TEXTURE_CUBEMAP ? 6 : 1);
       n++) {
    DDS_Surface* ddsSurface = &resultImage.Surfaces[resultImage.SurfaceCount];
    resultImage.SurfaceCount++;

    // calculate surface size
    uint32_t size = (*sizefunc)(width, height, resultImage) * depth;

    // calculate mipmap size
    ddsSurface->Size = size;
    ddsSurface->Width = width;
    ddsSurface->Height = height;
    ddsSurface->Depth = depth;
    ddsSurface->Pixels = cursor;
    cursor += size;

    if (flipImage) {
      flip(ddsSurface, resultImage);
    }

    uint32_t w = clamp_size(width >> 1);
    uint32_t h = clamp_size(height >> 1);
    uint32_t d = clamp_size(depth >> 1);

    // store number of mipmaps
    uint32_t numMipmaps = ddsh->MipMapCount;

    // number of mipmaps in file includes main surface so decrease count
    // by one
    if (numMipmaps != 0)
      numMipmaps--;

    // load all mipmaps for current surface
    for (uint32_t i = 0; i < numMipmaps && (w || h); i++) {
      DDS_Surface* mipmapSurface =
        &resultImage.Surfaces[resultImage.SurfaceCount];
      resultImage.SurfaceCount++;

      // calculate mipmap size
      mipmapSurface->Size = (*sizefunc)(w, h, resultImage) * d;
      mipmapSurface->Width = w;
      mipmapSurface->Height = h;
      mipmapSurface->Depth = d;
      mipmapSurface->Pixels = cursor;
      cursor += size;

      if (flipImage) {
        flip(mipmapSurface, resultImage);
      }

      // shrink to next power of 2
      w = clamp_size(w >> 1);
      h = clamp_size(h >> 1);
      d = clamp_size(d >> 1);
    }
  }

  /* std::swap cubemaps on y axis (since image is flipped in OGL)
  if (resultImage.Type == DDS_TEXTURE_CUBEMAP && flipImage) {
      tmp = Images[3];
      Images[3] = Images[2];
      Images[2] = tmp;
  }TODO: Figure this shit out*/

  return resultImage;
}
}