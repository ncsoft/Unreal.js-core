#include "JavascriptEditor.h"
#include "JavascriptEditorViewport.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"

#define LOCTEXT_NAMESPACE "JavascriptEditor"


#if WITH_EDITOR
class FJavascriptEditorViewportClient : public FEditorViewportClient
{
public:
	/** Constructor */
	explicit FJavascriptEditorViewportClient(FPreviewScene& InPreviewScene, const TWeakPtr<class SEditorViewport>& InEditorViewportWidget = nullptr)
		: FEditorViewportClient(nullptr,&InPreviewScene,InEditorViewportWidget)
	{		
	}
	~FJavascriptEditorViewportClient()
	{}
};

class SAutoRefreshEditorViewport : public SEditorViewport
{
	SLATE_BEGIN_ARGS(SAutoRefreshEditorViewport)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SEditorViewport::Construct(
			SEditorViewport::FArguments()
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
				.AddMetaData<FTagMetaData>(TEXT("JavascriptEditor.Viewport"))
			);
	}

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override
	{
		EditorViewportClient = MakeShareable(new FJavascriptEditorViewportClient(PreviewScene,SharedThis(this)));

		return EditorViewportClient.ToSharedRef();
	}

	void Redraw()
	{
		SceneViewport->InvalidateDisplay();
	}

public:
	TSharedPtr<FJavascriptEditorViewportClient> EditorViewportClient;
	
	/** preview scene */
	FPreviewScene PreviewScene;
};


TSharedRef<SWidget> UJavascriptEditorViewport::RebuildWidget()
{
	if (IsDesignTime())
	{
		return BuildDesignTimeWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EditorViewport", "EditorViewport"))
			]);
	}
	else
	{
		ViewportWidget = SNew(SAutoRefreshEditorViewport);
		
		return BuildDesignTimeWidget(ViewportWidget.ToSharedRef());
	}
}
#endif

UWorld* UJavascriptEditorViewport::GetViewportWorld() const
{
#if WITH_EDITOR
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->PreviewScene.GetWorld();
	}
#endif
	return nullptr;
}

void UJavascriptEditorViewport::Redraw()
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->Redraw();
	}
}
