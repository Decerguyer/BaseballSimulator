FROM public.ecr.aws/lambda/python:3.8

# Install the function's dependencies using file requirements.txt
# from your project folder.

COPY requirements.txt  .
RUN  pip3 install -r requirements.txt --target "${LAMBDA_TASK_ROOT}"
# Copy function code
COPY . ${LAMBDA_TASK_ROOT}

COPY ./node_modules/serverless-wsgi/serverless_wsgi.py ${LAMBDA_TASK_ROOT}/serverless_wsgi.py
COPY ./node_modules/serverless-wsgi/wsgi_handler.py ${LAMBDA_TASK_ROOT}/wsgi_handler.py
RUN echo -e '{"app": "app.app"}' > ${LAMBDA_TASK_ROOT}/.serverless-wsgi
# Set the CMD to your handler (could also be done as a parameter override outside of the Dockerfile)
CMD [ "wsgi_handler.handler" ]