#include <Unknwn.h>

#undef INTERFACE
#define INTERFACE IGraphicsCaptureItemInterop
DECLARE_INTERFACE_IID_(IGraphicsCaptureItemInterop, IUnknown, "3628E81B-3CAC-4C60-B7F4-23CE0E0C3356")
{
	IFACEMETHOD(CreateForWindow)(
		HWND window,
		REFIID riid,
		_COM_Outptr_ void** result
		) PURE;

	IFACEMETHOD(CreateForMonitor)(
		HMONITOR monitor,
		REFIID riid,
		_COM_Outptr_ void** result
		) PURE;

};

#include <inspectable.h>
#include "WindowCapture.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <windows.graphics.capture.h>
//#include "d3dHelpers.h"
#include "direct3d11.interop.h"

using namespace winrt;
using namespace Windows;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Foundation::Numerics; 


WindowCapture::WindowCapture(ID3D11Device* d3dDevice)
{
	//m_d3dDevice = CreateD3DDevice();
	m_d3dDevice.copy_from(d3dDevice);
	m_d3dDevice->GetImmediateContext(m_d3dContext.put());

	auto dxgiDevice = m_d3dDevice.as<IDXGIDevice>();
	m_device = CreateDirect3DDevice(dxgiDevice.get());

	m_hwnd = NULL;
	m_lastFrameArrived = 0;
}

void WindowCapture::CaptureMonitor(HMONITOR monitor)
{
	// CreateCaptureItem  
	auto activation_factory = winrt::get_activation_factory<GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
 
	interop_factory->CreateForMonitor(monitor, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(m_item)));

	// 
	auto size = m_item.Size();

	m_sharedHandle = OutMgr.GetSharedHandle();
	m_d3dDevice->OpenSharedResource(m_sharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_sharedSurface));

	// Create framepool, define pixel format (DXGI_FORMAT_B8G8R8A8_UNORM), and frame size. 
	m_framePool = Direct3D11CaptureFramePool::Create(
		m_device,
		DirectXPixelFormat::B8G8R8A8UIntNormalized,
		2,
		size);
	m_session = m_framePool.CreateCaptureSession(m_item);
	m_lastSize = size;
	m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &WindowCapture::OnFrameArrived });
	 
}
 
// Start sending capture frames
void WindowCapture::StartCapture()
{
    CheckClosed();
    m_session.StartCapture(); 
}
  
void WindowCapture::OnFrameArrived(
    Direct3D11CaptureFramePool const& sender,
    winrt::Windows::Foundation::IInspectable const&)
{
    auto frame = sender.TryGetNextFrame();
	auto frameContentSize = frame.ContentSize();
	DWORD dwTime = ::timeGetTime();
	if(dwTime - m_lastFrameArrived > 30)
    {  
		auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
        m_d3dContext->CopyResource(m_sharedSurface, frameSurface.get());
		m_d3dContext->Flush();

		OutMgr.UpdateWindow();

		m_lastFrameArrived = dwTime;
    }

}

// Process captured frames
void WindowCapture::Close()
{
	auto expected = false;
	if (m_closed.compare_exchange_strong(expected, true))
	{
		m_frameArrived.revoke();
		m_framePool.Close();
		m_session.Close();

		m_sharedSurface = nullptr;
		m_framePool = nullptr;
		m_session = nullptr;
		m_item = nullptr;

		m_d3dDevice = nullptr;
		m_d3dContext = nullptr;
	}
}
