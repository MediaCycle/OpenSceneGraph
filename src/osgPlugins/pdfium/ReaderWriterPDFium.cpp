/* -*-c++-*- OpenSceneGraph - Copyright (C) 2021 Christian Frisson
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgWidget/PdfReader>
#include <osgDB/ReaderWriter>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "samples/pdfium_test.h"

#include <sstream>

/** Implementation of class for interfacing with implementation of PDF reader.*/
class PdfiumImage : public osgWidget::PdfImage
{
    public:

        PdfiumImage():
            osgWidget::PdfImage(),
            _imageBuffer(0),
            _pageCount(0) {
                _reader = new pdfium_reader();
            }

        bool open(const std::string& fileName)
        {
            _fileName = fileName;
            _pageCount = 0;
            return this->page(0);
        }

        virtual int getNumOfPages() {
            return _pageCount;
        }

        virtual bool page(int pageNum) {

            std::vector<std::string> args;
            // Placeholder for command-line app path as first argument
            args.push_back("executable path");
            // Specify page number(s) to read
            std::stringstream pages;
            pages << "--pages=" << pageNum;
            args.push_back(pages.str());
            // List filename(s) as last argument(s)
            args.push_back(_fileName);
            
            int status = _reader->run(args);

            if(status != 1){

                int width = _reader->getWidth();
                int height = _reader->getHeight();
                int stride = _reader->getStride();
                _pageCount = _reader->getPageCount();
                FPDF_BITMAP bitmap = _reader->getBitmap();

                int s = width;
                int t = height;
                int r = 1;
                GLint internalTextureformat = GL_RGB;
                GLenum pixelFormat = GL_RGB;
                GLenum type = GL_UNSIGNED_BYTE;

                // From PDFium samples/pdfium_test_write_helper.cc WritePpm
                // Source data is B, G, R, unused.
                // Dest data is R, G, B.
                //const uint8_t* 
                _imageBuffer = reinterpret_cast<const uint8_t*>(FPDFBitmap_GetBuffer(bitmap));
                int out_len = width * height;
                out_len *= 3;
                _imageData = std::vector<unsigned char> (out_len,0);

                for (int h = 0; h < height; ++h) {
                    const uint8_t* src_line = _imageBuffer + (stride * h);
                    uint8_t* dest_line = _imageData.data() + (width * h * 3);
                    for (int w = 0; w < width; ++w) {
                        // R
                        dest_line[w * 3] = src_line[(w * 4) + 2];
                        // G
                        dest_line[(w * 3) + 1] = src_line[(w * 4) + 1];
                        // B
                        dest_line[(w * 3) + 2] = src_line[w * 4];
                    }
                }

                AllocationMode mode = osg::Image::NO_DELETE;
                this->setImage(s,t,r,internalTextureformat,pixelFormat,type,_imageData.data(),mode);
                this->setOrigin(osg::Image::TOP_LEFT);

                return true;
            }

            return false;
        }

    protected:
        std::string _fileName;
        pdfium_reader* _reader;
        std::vector<unsigned char> _imageData;
        const uint8_t* _imageBuffer;
        int _pageCount;

        virtual ~PdfiumImage() {
            delete _reader;
            _reader = 0;
            _imageBuffer = 0;
        }
};

/** Implementation of class for reading and writing of non native formats. */
class ReaderWriterPDFium : public osgDB::ReaderWriter
{
    public:

        ReaderWriterPDFium():
            osgDB::ReaderWriter() 
            {
                supportsExtension("pdf","PDFium PDF plugin");
            }
        
        virtual const char* className() const { return "PDFium PDF plugin"; }

        virtual ~ReaderWriterPDFium() {}

        virtual ReadResult readObject(const std::string& fileName, const Options* options) const 
        { 
            return readImage(fileName, options); 
        }
        
        virtual ReadResult readImage(const std::string& fileName, const Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string _fileName = osgDB::findDataFile( fileName, options );
            if (_fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osg::ref_ptr<PdfiumImage> image = new PdfiumImage;
            if(!image)
            {
                return ReadResult::ERROR_IN_READING_FILE;
            } 
            if(!image->open(fileName))
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            return image.get(); 
        }

        virtual ReadResult readNode(const std::string& fileName, const Options* options) const
        {
            ReadResult readResult = readImage(fileName, options);
            if (!readResult.validImage()) return readResult;

            osg::ref_ptr<osgWidget::PdfReader> pdfReader = new osgWidget::PdfReader();
            if (pdfReader->assign(dynamic_cast<osgWidget::PdfImage*>(readResult.getImage())))
            {
                return pdfReader.release();
            }
            else
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(pdf, ReaderWriterPDFium)
