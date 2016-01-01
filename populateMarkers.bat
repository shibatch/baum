@cd %~dp0
@for %%I in (0) do @(
  @for %%J in (0,1,2,3,4,5,6,7,8,9) do @(
    @for %%K in (0,1,2,3,4,5,6,7,8,9) do @(
      @echo Creating markers\baum.%%I%%J%%K.html
      @bin\baummarker %%I%%J%%K
      @move output.html markers\baum.%%I%%J%%K.html >nul
    )
  )
)
