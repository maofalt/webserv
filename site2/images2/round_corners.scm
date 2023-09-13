(define (script-round-corners filename-in filename-out)
  (let* ((image    (car (gimp-file-load RUN-NONINTERACTIVE filename-in "")))
         (width (car (gimp-image-width image)))
		 (height (car (gimp-image-height image))))
	
	 (gimp-image-crop image 400 400 (/ (* width 3) 13) (/ height 12))

     (let* ((layer (car (gimp-layer-copy (car (gimp-image-get-active-layer image)) TRUE))))
       (gimp-image-add-layer image layer 0)
       (gimp-image-set-active-layer image layer))

     (script-fu-round-corners image (car (gimp-image-get-active-layer image)) 50 FALSE 8 8 15 TRUE FALSE)
	 (gimp-image-flatten image)
     (gimp-file-save   RUN-NONINTERACTIVE image (car (gimp-image-get-active-drawable image)) filename-out "")
  )
)
